#pragma once

#include "consolearea.hpp"
#include "utils.hpp"
#include <functional>
#include <typeinfo>
#include <memory>
#include <variant>
#include <vector>




namespace escparse
{



// --------------------------------- API ---------------------------------


class BaseRule;
using ArgSupport = std::variant<int, std::string>;
class ParseContext;
class BaseFinalFunction;
using ParseResult = std::variant<const BaseFinalFunction*, const BaseRule*>;


class BaseFinalFunction
{
public:
    BaseFinalFunction(BaseFinalFunction&&) = default;
    BaseFinalFunction &operator=(BaseFinalFunction&&) = default;

public:
    BaseFinalFunction() {}
    virtual ~BaseFinalFunction() {}

public:
    virtual void call(std::vector<ArgSupport>& args) const = 0;

};


class BaseRuleContext
{
public:
    BaseRuleContext(BaseRuleContext&&) = default;
    BaseRuleContext &operator=(BaseRuleContext&&) = default;

public:
    BaseRuleContext() {}
    virtual ~BaseRuleContext() {}

public:
    virtual void addArgument(ParseContext& ctx) = 0;

};


enum class RuleType
{
    Container,
    Final
};


class BaseRule
{
public:
    BaseRule(BaseRule&&) = default;
    BaseRule &operator=(BaseRule&&) = default;

public:
    BaseRule() {}
    virtual ~BaseRule() {}

public:
    virtual RuleType type() const noexcept = 0;

public:
    virtual bool can_parse(char chr) const = 0;
    virtual bool pop_char() const noexcept { return true; }

};


class BaseContainerRule : public BaseRule
{
public:
    BaseContainerRule(BaseContainerRule&&) = default;
    BaseContainerRule &operator=(BaseContainerRule&&) = default;

public:
    BaseContainerRule() {}
    virtual ~BaseContainerRule() {}

public:
    RuleType type() const noexcept final { return RuleType::Container; }

    virtual std::string chr() const = 0;

public:
    virtual std::unique_ptr<BaseRuleContext> make_context() const = 0;
    virtual bool next(BaseRuleContext* ctx, char chr) const = 0;

public:
    virtual const std::vector<const BaseRule*>& child_rules() const = 0;

};


class BaseFinalRule : public BaseRule
{
public:
    BaseFinalRule(BaseFinalRule&&) = default;
    BaseFinalRule &operator=(BaseFinalRule&&) = default;

public:
    RuleType type() const noexcept final { return RuleType::Final; }

public:
    BaseFinalRule() {}
    virtual ~BaseFinalRule() {}

    virtual const char* info() const { return "unknown"; }

public:
    virtual const BaseFinalFunction* get_final() const = 0;

};


// --------------------------------- --- ---------------------------------


class ParseContext
{
public:
    ParseContext(const BaseContainerRule* root)
        : _root(root)
    {
        _current = _root;
    }

public:
    void next(char chr)
    {
        if (_invalid_stack) {
            _stack.clear();
            _invalid_stack = false;
        }
        _stack.push_back(chr);
        if (_context and _current->next(_context.get(), chr))
            ;
        else if (not _context and _current->can_parse(chr))
        {
            enter(_current);
        }
        else if (not _context)
        {
            _invalid_stack = true;
            throw std::runtime_error("_context == nullptr. Root escape rule corrupted?");
        }
        else {
            _context->addArgument(*this);
            auto& childs = _current->child_rules();
            for (auto child : childs) {
                if (not child->pop_char())
                    throw std::runtime_error("Internal error (no pop char reached)");
                if (child->can_parse(chr)) {
                    enter(child);
                    _current->next(_context.get(), chr);
                    return;
                }
            }
            std::string avail;
            for (auto child : childs) {
                auto info = (child->type() == RuleType::Container
                             ? dynamic_cast<const BaseContainerRule*>(child)->chr()
                             : dynamic_cast<const BaseFinalRule*>(child)->info());
                avail += info + ", ";
            }
            if (not avail.empty())
                avail.erase(avail.end() - 2, avail.end());
            _current = _root;
            _context = {};
            _invalid_stack = true;
            throw std::runtime_error("Can't find rule by chr '" + escape_string(std::string_view(&chr, 1)) + "', available: " + avail + ". skip.");
        }
    }

    const std::string& stack() const {
        return _stack;
    }

public:
    template<typename Arg>
    void addArgument(Arg&& arg)
    {
        _args.emplace_back(std::forward<Arg>(arg));
    }

private:
    void enter(const BaseRule* base)
    {
        if (base->type() == RuleType::Container) {
            auto current = static_cast<const BaseContainerRule*>(base);
            _current = current;
            _context = current->make_context();
            bool found = false;
            for (auto& child : current->child_rules()) {
                if (found)
                    throw std::runtime_error("Invalid rule, double no pop char");
                if (not child->pop_char()) {
                    enter(child);
                    found = true;
                    continue;
                }
            }
        }
        else {
            auto current = static_cast<const BaseFinalRule*>(base);
            current->get_final()->call(_args);
            _args.clear();
            _invalid_stack = true;
            Log->trace("ParseContext::enter Stack invalid '{}'", escape_string(_stack));
            _context = {};
            _current = _root;
        }
    }

private:
    const BaseContainerRule* _root;
    const BaseContainerRule* _current;
    std::unique_ptr<BaseRuleContext> _context;
    std::vector<ArgSupport> _args;
    std::string _stack;
    bool _invalid_stack = true;

};


// ------------------------------- Helpers -------------------------------


template<typename RuleContext>
class BaseRuleHelper : public BaseContainerRule
{
public:
    std::unique_ptr<BaseRuleContext> make_context() const override
    {
        return std::make_unique<RuleContext>();
    }

    virtual bool next(RuleContext* ctx, char chr) const = 0;
    bool next(BaseRuleContext* ctx, char chr) const override final
    {
        return next(static_cast<RuleContext*>(ctx), chr);
    }

};



template<typename T>
class SimpleContext : public BaseRuleContext
{
public:
    void addArgument(ParseContext& ctx) override
    {
        ctx.addArgument(value);
    }

public:
    T value = {};
};



template<>
class SimpleContext<void> : public BaseRuleContext
{
public:
    void addArgument(ParseContext& ctx) override {}
};


template<typename Root>
struct CommaInterator;

class RuleFactory
{
    RuleFactory() {}
public:
    template<typename T>
    RuleFactory(T&& v) : ptr(new T(std::move(v))) {}
    template<typename T>
    RuleFactory(CommaInterator<T>&& it);

    mutable std::unique_ptr<BaseRule> ptr;

};


struct CS_NoPopChar {};

constexpr CS_NoPopChar NoPopChar{};



template<typename... Args>
class FinalFunction : public BaseFinalFunction
{
public:
    FinalFunction(FinalFunction&&) = default;
    FinalFunction& operator=(FinalFunction&&) = default;
    FinalFunction(std::function<void(Args&&...)> func)
        : _func(func) {}

private:
    template<typename T>
    static T get(ArgSupport& arg) {
        if constexpr (std::is_convertible_v<int, T>)
            return std::forward<T>(std::get<int>(arg));
        else if constexpr (std::is_convertible_v<std::string, T>)
            return std::forward<T>(std::get<std::string>(arg));
        else
            throw std::runtime_error("Error can't select type");
    };

public:
    void call(std::vector<ArgSupport>& args) const override
    {
        using Tuple = std::tuple<Args...>;
        auto call = [&]<size_t... index>(std::index_sequence<index...> seq) {
            _func(get<typename std::tuple_element<index, Tuple>::type>(args.at(index))...);
        };
        constexpr auto seq = std::make_index_sequence<sizeof...(Args)>();
        call(seq);
    }

private:
    std::function<void(Args&&...)> _func;

};


template<typename... Args>
class FinalRule final : public BaseFinalRule
{
public:
    FinalRule(char chr, std::function<void(Args&&...)>&& func = {}, const char* info = nullptr)
        : _func(func),
          _chr(chr),
          _info(info ? info + std::string(typeid(void(*)(Args...)).name()) : "<unknown>" + std::string(typeid(void(*)(Args...)).name())) {}
    FinalRule(CS_NoPopChar, std::function<void(Args&&...)>&& func = {}, const char* info = nullptr)
        : _func(func),
          _chr('\0'),
          _info(info ? info + std::string(typeid(void(*)(Args...)).name()) : "<unknown>" + std::string(typeid(void(*)(Args...)).name())) {}

    template<typename Functor, typename... Args_>
    FinalRule(char chr, Functor* fn, void (Functor::* mem)(Args_...), const char* info = nullptr)
        : _func([fn, mem](Args&&... args) { (fn->*mem)(std::forward<Args_>(args)...); }),
          _chr(chr),
          _info(info ? std::string(info) + ": " + typeid(fn->*mem).name() : typeid(fn->*mem).name()) {}
    template<typename Functor, typename... Args_>
    FinalRule(CS_NoPopChar, Functor* fn, void (Functor::* mem)(Args_...), const char* info = nullptr)
        : _func([fn, mem](Args&&... args) { (fn->*mem)(std::forward<Args_>(args)...); }),
          _chr('\0'),
          _info(info ? std::string(info) + ": " + typeid(fn->*mem).name() : typeid(fn->*mem).name()) {}

public:
    bool pop_char() const noexcept override
    {
        return _chr != '\0';
    }

    const BaseFinalFunction* get_final() const override
    {
        return &_func;
    }

    bool can_parse(char chr) const override
    {
        return _chr == '\0' and _chr == chr;
    }

    const char* info() const override
    {
        return _info.c_str();
    }

public:
    template<typename Functor, typename... Args_>
    FinalRule& operator()(Functor* fn, void (Functor::* mem)(Args...)) &
    {
        _func = [fn, mem](Args&&... args) { (fn->*mem)(std::forward<Args_>(args)...); };
        return (FinalRule&)*this;
    }

    template<typename... Args_>
    FinalRule&& operator()(Args_&&... args) && { return std::move((*this)(std::forward<Args>(args)...)); }

public:
    FinalRule& operator[](std::function<void(Args&&...)>&& func) {
        _func = std::move(func);
        return *this;
    }

private:
    char _chr;
    FinalFunction<Args...> _func;
    std::string _info;

};



template<typename Base>
class RuleContainerProxy : public Base
{
public:
    using Derrived = RuleContainerProxy<Base>;
    using Base::Base;

    RuleContainerProxy(RuleContainerProxy&& m)
        : Base(std::move((Base&)m)), _rules(std::move(m._rules)), _rules_ptrs(std::move(m._rules_ptrs)) {}
    Derrived& operator=(Derrived&& m) = default;

public:
    template<typename... Args>
    Derrived& append(Args&&... args) &
    {
        auto append = [&]<typename T>(T&& arg) {
            _rules_ptrs.emplace_back(std::make_unique<std::remove_reference_t<T>>(std::move(arg)));
            _rules.push_back(_rules_ptrs.back().get());
        };
        (append(args), ...);
        return (Derrived&)*this;
    }

    Derrived& append(std::initializer_list<RuleFactory>&& init) &
    {
        auto append = [&](const RuleFactory& arg) {
            _rules_ptrs.emplace_back(std::move(arg.ptr));
            _rules.push_back(_rules_ptrs.back().get());
        };
        for (auto& v : init)
            append(v);
        return (Derrived&)*this;
    }

    Derrived&& append(std::initializer_list<RuleFactory>&& init) &&
    {
        return std::move((*this).append(std::move(init)));
    }

    template<typename... Args>
    Derrived&& append(Args&&... args) && { return std::move((*this).append(std::forward<Args>(args)...)); }

    template<typename Functor, typename... Args>
    Derrived& operator()(Functor* fn, void (Functor::* mem)(Args...), const char* info = nullptr) &
    {
        append<FinalRule<std::decay_t<Args>...>>(FinalRule<std::decay_t<Args>...>(NoPopChar, fn, mem, info));
        return (Derrived&)*this;
    }

    template<typename... Args>
    Derrived&& operator()(Args&&... args) && { return std::move((*this)(std::forward<Args>(args)...)); }

public:
    const std::vector<const BaseRule*>& child_rules() const override
    {
        return _rules;
    }

private:
    std::vector<const BaseRule*> _rules;
    std::vector<std::unique_ptr<BaseRule>> _rules_ptrs;

};


// ------------------------------- ------- -------------------------------



class TParseInt : public BaseRuleHelper<SimpleContext<int>>
{
public:
    TParseInt(TParseInt&&) = default;
    TParseInt() {}

    TParseInt& operator=(TParseInt&&) = default;

public:
    bool can_parse(char chr) const override
    {
        return '0' <= chr && chr <= '9';
    }

    bool next(SimpleContext<int>* ctx, char chr) const override
    {
        if (not can_parse(chr))
            return false;
        ctx->value *= 10;
        ctx->value += chr - '0';
        return true;
    }

    std::string chr() const override { return "<int>"; }

};



class TSkipChar : public BaseRuleHelper<SimpleContext<void>>
{
public:
    TSkipChar(TSkipChar&&) = default;
    TSkipChar(char chr) : _char(chr) {}

    TSkipChar& operator=(TSkipChar&&) = default;

public:
    bool can_parse(char chr) const override
    {
        return chr == _char;
    }

    bool next(SimpleContext<void>* ctx, char chr) const override
    {
        return false;
    }

    std::string chr() const override { return escape_string({&_char, 1}); }

private:
    char _char;

};



class TParseChar : public BaseRuleHelper<SimpleContext<uint8_t>>
{
public:
    TParseChar(TParseChar&&) = default;
    TParseChar() {}
    TParseChar(char chr) : _pred([chr](char c) { return c == chr; }), _info(escape_string({&chr, 1})) {}
    TParseChar(std::function<bool(char)>&& pred, const char* info = "<pred>") : _pred(std::move(pred)), _info(info) {}

    TParseChar& operator=(TParseChar&&) = default;

public:
    bool can_parse(char chr) const override
    {
        return not _pred or _pred(chr);
    }

    bool next(SimpleContext<uint8_t>* ctx, char chr) const override
    {
        ctx->value = chr;
        return false;
    }

    std::string chr() const override { return _info; }

private:
    std::function<bool(char)> _pred;
    std::string _info;

};


using ParseInt = RuleContainerProxy<TParseInt>;
using SkipChar = RuleContainerProxy<TSkipChar>;
using ParseChar = RuleContainerProxy<TParseChar>;



std::unique_ptr<BaseContainerRule> make_xterm_rules();



inline std::vector<std::string> dump2(const BaseRule* root)
{
    /* \e[<int>;<int>;<int>M -> ...
     */
    if (root->type() == RuleType::Container)
    {
        auto rule = dynamic_cast<const BaseContainerRule*>(root);
        std::vector<std::string> content;
        for (auto& child : rule->child_rules()) {
            for (auto& l : dump2(child))
                content.emplace_back(rule->chr() + std::move(l));
        }
        return content;
    }
    else
    {
        auto rule = dynamic_cast<const BaseFinalRule*>(root);
        return {std::string(" -> ") + rule->info()};
    }
}



inline std::string dump(const BaseRule* root)
{
    std::string res;
    for (auto& l : dump2(root))
        res += l + "\n";
    return res;
}



template<typename Root>
struct CommaInterator : public Root
{
    CommaInterator(Root&& r) : Root(std::move(r)) {}
    std::function<RuleFactory(RuleFactory&&)> childs;
};


template<typename T>
inline RuleFactory::RuleFactory(CommaInterator<T>&& it)
{
    auto ptr = std::make_unique<T>(std::move((T&)it));
    ptr->append({std::move(it.childs({}))});
    this->ptr = std::move(ptr);
}


template<typename Left, typename Right>
CommaInterator<RuleContainerProxy<Left>> operator,(RuleContainerProxy<Left>&& l, Right&& r)
{
    CommaInterator<RuleContainerProxy<Left>> i{std::move(l)};
    i.childs = [r=std::make_shared<Right>(std::move(r))](RuleFactory&& f)
            mutable { r->append({std::move(f)}); return RuleFactory{std::move(*r)}; };
    return i;
}


template<typename Left, typename Right>
CommaInterator<Left> operator,(CommaInterator<Left>&& l, Right&& r)
{
    l.childs = [c=std::move(l.childs), r=std::make_shared<Right>(std::move(r))](RuleFactory&& f)
            mutable { if (f.ptr) r->append({std::move(f)}); return c(RuleFactory(std::move(*r))); };
    return l;
}



}
