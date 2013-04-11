'''
Wrapper for Amazon SWF (Simple Workflows).

References:
http://docs.aws.amazon.com/amazonswf/latest/developerguide
http://docs.aws.amazon.com/amazonswf/latest/apireference
http://boto.readthedocs.org/en/2.6.0/ref/swf.html
https://github.com/boto/boto/blob/develop/boto/swf/layer1.py
https://github.com/boto/boto/blob/develop/boto/swf/layer1_decisions.py
https://github.com/boto/boto/blob/develop/boto/swf/exceptions.py
'''

import functools
import simplejson as json
import boto.swf
import boto.swf.layer1
import boto.swf.layer1_decisions
import boto.swf.exceptions
import pomagma.util
import pomagma.store


DOMAIN = 'pomagma'
VERSION = '1.0'
SWF = boto.swf.layer1.Layer1()


#-----------------------------------------------------------------------------
# Admin


def register_domain():
    print 'Registering domain'
    try:
        SWF.register_domain(
                name=DOMAIN,
                workflow_execution_retention_period_in_days=90)
    except boto.swf.exceptions.SWFDomainAlreadyExistsError:
        pass


def register_activity_type(name):
    activity_type = '{}ActivityType'.format(name)
    print 'Registering activity type {}'.format(activity_type)
    try:
        SWF.register_activity_type(DOMAIN, activity_type, VERSION)
    except boto.swf.exceptions.SWFTypeAlreadyExistsError:
        pass


def register_workflow_type(name, task_list):
    workflow_type = '{}ActivityType'.format(name)
    print 'Registering workflow type {}'.format(workflow_type)
    try:
        SWF.register_workflow_type(DOMAIN, workflow_type, VERSION, task_list)
    except boto.swf.exceptions.SWFTypeAlreadyExistsError:
        pass


def start_workflow_execution(workflow_id, workflow, version):
    SWF.start_workflow_execution(
        domain=DOMAIN,
        workflow_id=workflow_id,
        workflow_name=workflow,
        version=VERSION)

#-----------------------------------------------------------------------------
# Decisions

def poll_decision_task(name):
    task_list = '{}TaskList'.format(name)
    while True:
        response = SWF.poll_for_decision_task(DOMAIN, task_list)
        if response['taskToken']:
            print 'Decision Task: {}'.format(response)
            return response


def decide_to_schedule(decision_task, activity_type, input=None):
    task_token = decision_task['taskToken']
    decisions = boto.swf.layer1_decisions.Layer1Decisions()
    task_list = '{}TaskList'.format(activity_type)
    activity_id = pomagma.util.random_uuid()
    decisions.schedule_activity_task(
        activity_id=activity_id,
        activity_type_name='{}ActivityType'.format(activity_type),
        activity_type_version=VERSION,
        task_list=task_list,
        input=input,
        )
    SWF.respond_decision_task_completed(task_token, decisions)


def decide_to_complete(decision_task):
    task_token = decision_task['taskToken']
    decisions = boto.swf.layer1_decisions.Layer1Decisions()
    decisions.complete_workflow_execution()
    SWF.respond_decision_task_completed(task_token, decisions)


#-----------------------------------------------------------------------------
# Activities

def poll_activity_task(name):
    task_list = '{}TaskList'.format(name)
    while True:
        response = SWF.poll_for_activity_task(DOMAIN, task_list)
        if response['taskToken']:
            print 'Activity Task: {}'.format(response)
            return response


def finish_activity_task(task, result=None):
    SWF.respond_activity_task_completed(task['taskToken'], result=result)


def fail_activity_task(task, reason=None, details=None):
    SWF.respond_activity_task_failed(
        task['taskToken'],
        reason=reason,
        details=details)


def reproducible(module):
    '''
    Create decorator to run activities inreproducible environment.

    TODO send log file in error message
    '''
    def reproducible_(fun):
        @functools.wraps(fun)
        def reproducible_fun(task):
            try:
                with pomagma.util.chdir(pomagma.util.DATA):
                    result = fun(task)
                finish_activity_task(task, result)
            except Exception, exc:
                reason = str(exc)
                task_str = json.dumps(task, indent=4, sort_keys=True)
                details = '\n'.join([
                    '#!/usr/bin/env python',
                    'import json',
                    'import pomagma.util',
                    'import pomagma.workflow.grow',
                    'pomagma.store.get(log_file)',
                    'task = json.load({r})'.format(task_str),
                    'with pomagma.util.chdir(pomagma.util.DATA):',
                    '    {}.{}({})'.format(module, fun.__name__, task),
                    ])
                fail_activity_task(task, reason, details)
        return reproducible_fun
    return reproducible_