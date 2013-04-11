'''
Wrapper for Amazon S3 and bzip2 compression.

References:
http://boto.readthedocs.org/en/latest/ref/s3.html
'''

import os
import sys
import subprocess
import boto


def try_connect_s3(bucket):
    try:
        return boto.connect_s3().get_bucket(bucket)
    except boto.exception.NoAuthHandlerFound:
        sys.stderr.write(
            'WARNING failed to connect to s3 bucket {}\n'.format(bucket))
        sys.stderr.flush()
        return None


BUCKET = try_connect_s3('pomagma')


def s3_put(filename):
    key = BUCKET.new_key(filename)
    key.set_contents_from_filename(filename)


def s3_get(filename):
    key = BUCKET.get_key(filename)
    key.get_contents_to_filename(filename)


def s3_listdir(prefix):
    return BUCKET.list(prefix)


def s3_remove(filename):
    key = BUCKET.get_key(filename)
    if key is not None:
        key.delete()


def s3_exists(filename):
    key = BUCKET.get_key(filename)
    return key is not None


def bzip2(filename):
    subprocess.check_call(['bzip2', '--keep', '--force', filename])
    filename_bz2 = '{}.bz2'.format(filename)
    return filename_bz2


def bunzip2(filename_bz2):
    assert filename_bz2[-4:] == '.bz2', filename_bz2
    subprocess.check_call(['bunzip2', '--keep', '--force', filename_bz2])
    return filename_bz2[:-4]


def get(filename):
    if os.path.exists(filename):
        return filename
    filename_bz2 = '{}.bz2'.format(filename)
    if os.path.exists(filename_bz2):
        return bunzip2(filename_bz2)
    s3_get(filename_bz2)
    return bunzip2(filename_bz2)


def put(filename):
    filename_bz2 = bzip2(filename)
    s3_put(filename_bz2)


def listdir(prefix):
    for filename_bz2 in s3_listdir(prefix):
        assert filename_bz2[-4:] == '.bz2', filename_bz2
        yield filename_bz2[:-4].lstrip('/')


def remove(filename):
    if os.path.exists(filename):
        os.remove(filename)
    filename_bz2 = '{}.bz2'.format(filename)
    if os.path.exists(filename_bz2):
        os.remove(filename_bz2)
    s3_remove(filename_bz2)


def remove_local(filename):
    if os.path.exists(filename):
        os.remove(filename)
    filename_bz2 = '{}.bz2'.format(filename)
    if os.path.exists(filename_bz2):
        os.remove(filename_bz2)
