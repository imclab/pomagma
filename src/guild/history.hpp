
namespace pomagma
{

class History
{
public:

private:
    std::set<std::pair<Expr, Timestamp>> m_snapshots;

    // TODO How to make this more general, and not hard-code each function?
    std::unordered_map<Expr, Expr> m_quote;
    std::unordered_map<std::pair<Expr, Expr>, Expr> m_apply;
    std::unordered_map<std::pair<Expr, Expr>, Expr> m_compose;
    std::unordered_map<std::pair<Expr, Expr>, Expr> m_join;

    struct Abstraction
    {
        Var & var;
        Expr & body;
    };
    std::unordered_map<Abstraction, Expr> m_abstract;
    std::unordered_map<QuoteAbstraction, Expr> m_quote_abstract;
    std::unordered_map<BoxAbstraction, Expr> m_box_abstract;

    struct Definition
    {
        Var & var;
        Expr & defn;
        Expr & body;
    };
    std::unordered_map<std::pair<Var, Expr>, Expr> m_let;
    std::unordered_map<std::pair<Var, Expr>, Expr> m_quote_let;
    std::unordered_map<std::pair<Var, Expr>, Expr> m_box_let;
};

} // namespace pomagma
