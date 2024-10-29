#include "parser.hpp"
#include "ast.hpp"
#include "buaa.hpp"
#include "logger.hpp"
#include "token.hpp"
#include "type.hpp"
#include <initializer_list>
#include <memory>
#include <tuple>
#include <vector>

namespace blang {

namespace frontend {

const std::map<char, char> Parser::_escape_character_map
= {
    {'a', '\a'}, {'b', '\b'}, {'t', '\t'},
    {'n', '\n'}, {'v', '\v'}, {'f', '\f'}, 
    {'"', '\"'}, {'\'', '\''}, {'\\', '\\'},
    {'0', '\0'},
    };

Parser::Parser(std::shared_ptr<Logger> logger) {
    _logger = logger;
    _pos = 0;
}

std::string Parser::parseString(const std::string in) {
    std::string ret = "";
    for (auto iter = in.begin(), end = in.end(); iter != end; iter++) {
        if (*iter == '\\') {
            iter++;
            ret += _escape_character_map.find(*iter)->second;
            continue;
        }
        ret += (*iter);
    }

    return ret;
}

int32_t Parser::parseInt(const std::string in) {
    return std::stoll(in);
}

char Parser::parseChar(const std::string in) {
    if (in[0] == '\\') {
        return _escape_character_map.find(in[1])->second;
    } else {
        return in[0];
    }
}

Type* Parser::token2Type(Token token) {
    Type* ret;
    switch (token.type) {
        case INT:
            ret = IntType::get();
            break;
        case CHAR:
            ret = CharType::get();
            break;
        case VOID:
            ret = VoidType::get();
            break;
        default:
            ret = nullptr;
            break;
    }

    return ret;
}

bool Parser::match(std::initializer_list<TokenType> list) {
    uint32_t count = 0;
    for (const auto& type : list) {
        if (peak(count).type != type) {
            return false;
        }
        count++;
    }

    return true;
}

void Parser::update(std::shared_ptr<ParseResult> buffer, std::shared_ptr<ParseResult> result) {
    for (auto& log : result->log_buffer) {
        buffer->log(log);
    }
    for (auto& error : result->error_buffer) {
        buffer->logError(error);
    }
}

#define CHECK_AND_STEP(type, result) if (check(type)) result->log(step()); else return result

std::shared_ptr<CompNode> Parser::parse(std::shared_ptr<std::vector<Token>> tokens) {
    _pos = 0;
    this->_tokens = tokens;
    auto result = parseCompUnit();
    for (auto& log : result->log_buffer) {
        _logger->log(log);
    }
    for (auto& error : result->error_buffer) {
        _logger->logError(error);
    }

    return result->get<CompNode>();
}

template<typename T>
std::shared_ptr<T> Parser::tryParse(ParseFuncPtr func, std::shared_ptr<ParseResult> buffer) {
    auto pos = _pos;
    if (auto result = (this->*func)(); result->get<T>()) {
        update(buffer, result);
        return result->get<T>();
    } else {
        revert(pos);
    }

    return nullptr;
}

template<typename T>
std::shared_ptr<T> Parser::tryParse(ParseTypeFuncPtr func, bool is_const, std::shared_ptr<ParseResult> buffer) {
    auto pos = _pos;
    if (auto result = (this->*func)(is_const); result->get<T>()) {
        update(buffer, result);
        return result->get<T>();
    } else {
        revert(pos);
    }

    return nullptr;
}

std::shared_ptr<ParseResult> Parser::parseCompUnit() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    auto comp_unit = std::shared_ptr<CompNode>();
    if (auto func_def = tryParse<FuncDefNode>(&Parser::parseFuncDef, ret); func_def) {
        comp_unit = std::make_shared<CompNode>(sline, func_def);
    } else if (auto main_func_def = tryParse<MainNode>(&Parser::parseMainFuncDef, ret); main_func_def) {
        comp_unit = std::make_shared<CompNode>(sline, main_func_def);
    } else if (auto decl = tryParse<DeclNode>(&Parser::parseDecl, ret); decl) {
        comp_unit = std::make_shared<CompNode>(sline, decl);
    } else {
        return ret;
    }

    while (!atEnd()) {
        sline = line();
        if (auto func_def = tryParse<FuncDefNode>(&Parser::parseFuncDef, ret); func_def) {
            comp_unit = std::make_shared<CompNode>(sline, func_def, comp_unit);
        } else if (auto decl = tryParse<DeclNode>(&Parser::parseDecl, ret); decl) {
            comp_unit = std::make_shared<CompNode>(sline, decl, comp_unit);
        } else if (auto main_func_def = tryParse<MainNode>(&Parser::parseMainFuncDef, ret); main_func_def) {
            comp_unit = std::make_shared<CompNode>(sline, main_func_def, comp_unit);
        } else {
            return ret;
        }
    }

    ret->node = comp_unit;

    ret->log(std::make_shared<ParserLog>(line(), "CompUnit"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseMainFuncDef() {
    auto ret = std::make_shared<ParseResult>();

    Type* type = token2Type(current());
    if (!type) {
        return ret; // abort
    }
    ret->log(step());   // type
    auto sline = line();
    CHECK_AND_STEP(MAIN, ret);
    CHECK_AND_STEP(LEFT_BRACE, ret);
    if (check(RIGHT_BRACE)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "right brace not found!", buaa::ERROR_MISSING_BRACE));
    }

    auto block = tryParse<BlockNode>(&Parser::parseBlock, ret);
    ret->log(std::make_shared<ParserLog>(line(), "MainFuncDef"));

    ret->node = std::make_shared<MainNode>(sline, block);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseFuncDef() {
    auto ret = std::make_shared<ParseResult>();

    Type* type = token2Type(current());
    if (!type) {
        return ret; // abort
    }
    ret->log(step());   // type
    if (!check(IDENT)) {
        return ret; // abort
    }
    ret->log(std::make_shared<ParserLog>(line(), "FuncType"));
    auto ident = current().value;
    auto sline = line();
    ret->log(step());   // ident

    auto params = std::shared_ptr<FuncFParamsNode>();
    CHECK_AND_STEP(LEFT_BRACE, ret);
    if (!check(RIGHT_BRACE)) {
        if (auto result = tryParse<FuncFParamsNode>(&Parser::parseFuncFParams, ret); result) {
            params = result;
        }
        if (check(RIGHT_BRACE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "right brace missing!", buaa::ERROR_MISSING_BRACE));
        }
    } else {
        ret->log(step());
    }

    auto block = tryParse<BlockNode>(&Parser::parseBlock, ret);

    ret->log(std::make_shared<ParserLog>(line(), "FuncDef"));

    ret->node = std::make_shared<FuncDefNode>(sline, type, ident, block, params);

    return ret;
}

std::tuple<Type*, std::string> Parser::parseFuncParam(std::shared_ptr<ParseResult> result) {
    auto type = token2Type(current());
    result->log(step());   // type
    auto ident = current().value;
    result->log(step());   // ident
    if (check(LEFT_SQUARE)) {
        result->log(step());
        if (check(RIGHT_SQUARE)) {
            result->log(step());
        } else {
            result->logError(std::make_shared<ErrorLog>(last().line, "no right square!", buaa::ERROR_MISSING_SQUARE));
        }
        type = PtrType::get(type);
    }

    result->log(std::make_shared<ParserLog>(line(), "FuncFParam"));

    return std::make_tuple(type, ident);
}

std::shared_ptr<ParseResult> Parser::parseFuncFParams() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();

    auto params = std::vector<std::tuple<Type*, std::string>>();
    if (check(LEFT_BRAKET)) {
        return ret; // abort
    }
    params.push_back(parseFuncParam(ret));

    while (check(COMMA)) {
        ret->log(step());
        params.push_back(parseFuncParam(ret));
    }

    ret->log(std::make_shared<ParserLog>(line(), "FuncFParams"));

    ret->node = std::make_shared<FuncFParamsNode>(line(), params);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseBlock() {
    auto ret = std::make_shared<ParseResult>();

    CHECK_AND_STEP(LEFT_BRAKET, ret);

    auto items = std::vector<std::shared_ptr<BlockItemNode>>();
    while (current().type != RIGHT_BRAKET) {
        if (atEnd()) {
            return ret; // abort
        }
        items.push_back(tryParse<BlockItemNode>(&Parser::parseBlockItem, ret));
    }
    CHECK_AND_STEP(RIGHT_BRAKET, ret);

    ret->log(std::make_shared<ParserLog>(line(), "Block"));

    ret->node = std::make_shared<BlockNode>(last().line, items);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseBlockItem() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (auto decl = tryParse<DeclNode>(&Parser::parseDecl, ret); decl) {
        ret->node = std::make_shared<BlockItemNode>(sline, decl);
    } else if (auto stmt = tryParse<StmtNode>(&Parser::parseStmt, ret); stmt) {
        ret->node = std::make_shared<BlockItemNode>(sline, stmt);
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseExpStmt() {
    auto ret = std::make_shared<ParseResult>();

    if (check(SEMICOLON)) {
        ret->log(step());
        ret->node = std::make_shared<ExpStmtNode>(last().line);
    } else {
        auto sline = line();
        if (auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret); exp) {
            if (check(SEMICOLON)) {
                ret->log(step());
            } else {
                ret->logError(std::make_shared<ErrorLog>(last().line, "exp stmt no semi", buaa::ERROR_MISSING_SEMICOLON));
            }
            ret->node = std::make_shared<ExpStmtNode>(sline, exp);
        } else {
            return ret; // abort
        }
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseBlockStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (auto block = tryParse<BlockNode>(&Parser::parseBlock, ret); block) {
        ret->node = std::make_shared<BlockStmtNode>(sline, block);
    } else {
        return ret; // abort
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseIfStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    CHECK_AND_STEP(IF, ret);
    CHECK_AND_STEP(LEFT_BRACE, ret);

    auto cond = tryParse<ExpNode>(&Parser::parseCondExp, ret);
    if (check(RIGHT_BRACE)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "if no right brace", buaa::ERROR_MISSING_BRACE));
    }

    auto if_stmt = tryParse<StmtNode>(&Parser::parseStmt, ret);
    auto else_stmt = std::shared_ptr<StmtNode>();
    if (check(ELSE)) {
        ret->log(step());
        else_stmt = tryParse<StmtNode>(&Parser::parseStmt, ret);
    }

    ret->node = std::make_shared<IfStmtNode>(sline, cond, if_stmt, else_stmt);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseForStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    CHECK_AND_STEP(FOR, ret);
    CHECK_AND_STEP(LEFT_BRACE, ret);

    auto for_in = std::shared_ptr<StmtNode>();
    auto cond = std::shared_ptr<ExpNode>();
    auto for_out = std::shared_ptr<StmtNode>();
    auto for_stmt = std::shared_ptr<StmtNode>();

    if (check(SEMICOLON)) {
        ret->log(step());
    } else {
        auto sline = line();
        auto lval = tryParse<LValNode>(&Parser::parseLVal, ret);
        CHECK_AND_STEP(ASSIGN, ret);
        auto rval = tryParse<RValNode>(&Parser::parseRVal, ret);
        for_in = std::make_shared<AssignStmtNode>(sline, lval, rval);
        ret->log(std::make_shared<ParserLog>(line(), "ForStmt"));
        CHECK_AND_STEP(SEMICOLON, ret);
    }
    if (check(SEMICOLON)) {
        ret->log(step());
    } else {
        cond = tryParse<ExpNode>(&Parser::parseCondExp, ret);
        CHECK_AND_STEP(SEMICOLON, ret);
    }
    if (!check(RIGHT_BRACE)) {
        auto sline = line();
        auto lval = tryParse<LValNode>(&Parser::parseLVal, ret);
        CHECK_AND_STEP(ASSIGN, ret);
        auto rval = tryParse<RValNode>(&Parser::parseRVal, ret);
        for_out = std::make_shared<AssignStmtNode>(sline, lval, rval);
        ret->log(std::make_shared<ParserLog>(line(), "ForStmt"));
        if (check(RIGHT_BRACE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "for no brace", buaa::ERROR_MISSING_BRACE));
        }
    } else {
        ret->log(step());
    }

    for_stmt = tryParse<StmtNode>(&Parser::parseStmt, ret);

    ret->node = std::make_shared<ForStmtNode>(sline, for_in, cond, for_out, for_stmt);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseReturnStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    CHECK_AND_STEP(RETURN, ret);

    if (auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret); exp) {
        ret->node = std::make_shared<ReturnStmtNode>(sline, exp);
    } else {
        ret->node = std::make_shared<ReturnStmtNode>(sline);
    }

    if (check(SEMICOLON)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "return no semi", buaa::ERROR_MISSING_SEMICOLON));
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parsePrintfStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    CHECK_AND_STEP(PRINTF, ret);
    CHECK_AND_STEP(LEFT_BRACE, ret);

    auto fmt = (parseString(current().value));
    ret->log(step());

    auto exps = std::vector<std::shared_ptr<ExpNode>>();
    while (check(COMMA)) {
        ret->log(step());
        exps.push_back(tryParse<ExpNode>(&Parser::parseExp, false, ret));
    }
    if (check(RIGHT_BRACE)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "printf no brace", buaa::ERROR_MISSING_BRACE));
    }
    if (check(SEMICOLON)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "printf no semi", buaa::ERROR_MISSING_SEMICOLON));
    }

    ret->node = std::make_shared<PrintfStmtNode>(sline, fmt, exps);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseAssignStmt() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();  
    if (auto lval = tryParse<LValNode>(&Parser::parseLVal, ret); lval) {
        CHECK_AND_STEP(ASSIGN, ret);
        auto rval = tryParse<RValNode>(&Parser::parseRVal, ret);
        if (check(SEMICOLON)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "assign no semi", buaa::ERROR_MISSING_SEMICOLON));
        }
        ret->node = std::make_shared<AssignStmtNode>(sline, lval, rval);
    } else {
        return ret; // abort
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseStmt() {
    auto ret = std::make_shared<ParseResult>();

    if (auto stmt = tryParse<AssignStmtNode>(&Parser::parseAssignStmt, ret); stmt) {
        ret->node = stmt;
    } else if (auto stmt = tryParse<ExpStmtNode>(&Parser::parseExpStmt, ret); stmt) {
        ret->node = stmt;
    } else if (auto stmt = tryParse<BlockStmtNode>(&Parser::parseBlockStmt, ret); stmt) {
        ret->node = stmt;
    } else if (auto stmt = tryParse<IfStmtNode>(&Parser::parseIfStmt, ret); stmt) {
        ret->node = stmt;
    } else if (auto stmt = tryParse<ForStmtNode>(&Parser::parseForStmt, ret); stmt) {
        ret->node = stmt;
    } else if (check(BREAK)) {
        auto sline = line();
        ret->log(step());
        if (check(SEMICOLON)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "break no semi", buaa::ERROR_MISSING_SEMICOLON));
        }
        ret->node = std::make_shared<BreakStmtNode>(sline);
    } else if (check(CONTINUE)) {
        auto sline = line();
        ret->log(step());
        if (check(SEMICOLON)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "break no semi", buaa::ERROR_MISSING_SEMICOLON));
        }
        ret->node = std::make_shared<BreakStmtNode>(sline);
    } else if (auto stmt = tryParse<ReturnStmtNode>(&Parser::parseReturnStmt, ret); stmt) {
        ret->node = stmt;
    } else if (auto stmt = tryParse<PrintfStmtNode>(&Parser::parsePrintfStmt, ret); stmt) {
        ret->node = stmt;
    } else {
        return ret; // abort
    }

    ret->log(std::make_shared<ParserLog>(line(), "Stmt"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseExp(bool is_const) {
    auto ret = std::make_shared<ParseResult>();

    ret->node = tryParse<ExpNode>(&Parser::parseAddExp, ret);

    if (is_const) {
        ret->log(std::make_shared<ParserLog>(line(), "ConstExp"));
    } else {
        ret->log(std::make_shared<ParserLog>(line(), "Exp"));
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseCondExp() {
    auto ret = std::make_shared<ParseResult>();

    ret->node = tryParse<ExpNode>(&Parser::parseLOrExp, ret);

    ret->log(std::make_shared<ParserLog>(line(), "Cond"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseLVal() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (!check(IDENT)) {
        return ret; // abort
    }

    auto ident = current().value;
    ret->log(step());

    auto exp = std::shared_ptr<ExpNode>();
    if (check(LEFT_SQUARE)) {
        ret->log(step());
        exp = tryParse<ExpNode>(&Parser::parseExp, false, ret);
        if (check(RIGHT_SQUARE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "lval mis square", buaa::ERROR_MISSING_SQUARE));
        }
    }

    ret->log(std::make_shared<ParserLog>(sline, "LVal"));

    ret->node = std::make_shared<LValNode>(sline, ident, exp);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parsePrimary() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (check(LEFT_BRACE)) {
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret);
        if (check(RIGHT_BRACE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "primary (exp) no brace", buaa::ERROR_MISSING_BRACE));
        }

        ret->node = std::make_shared<PrimaryExpNode>(sline, exp);
    } else if (auto value_n = tryParse<ValueNode>(&Parser::parseValue, ret); value_n) {
        ret->node = std::make_shared<PrimaryExpNode>(sline, value_n);
    } else if (auto lval = tryParse<LValNode>(&Parser::parseLVal, ret); lval) {
        ret->node = std::make_shared<PrimaryExpNode>(sline, lval);
    } else {
        return ret; // abort
    }

    ret->log(std::make_shared<ParserLog>(line(), "PrimaryExp"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseRVal() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret); exp) {
        ret->node = std::make_shared<RValNode>(sline, exp);
    } else if (check(GETINT) || check(GETCHAR)) {
        auto type = check(GETINT) ? RVAL_GETINT : RVAL_GETCHAR;
        ret->node = std::make_shared<RValNode>(sline, type);
        ret->log(step());
        CHECK_AND_STEP(LEFT_BRACE, ret);
        if (check(RIGHT_BRACE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "rval no brace", buaa::ERROR_MISSING_BRACE));
        }
    }

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseValue() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (check(CHAR_CONSTANT)) {
        ret->node = std::make_shared<ValueNode>(sline, parseChar(current().value));
        ret->log(step());
        ret->log(std::make_shared<ParserLog>(line(), "Character"));
    } else if (check(INT_CONSTANT)) {
        ret->node = std::make_shared<ValueNode>(sline, parseInt(current().value));
        ret->log(step());
        ret->log(std::make_shared<ParserLog>(line(), "Number"));
    }

    return ret;
}

Op Parser::parseUnaryOp(std::shared_ptr<ParseResult> buffer) {
    Op ret;

    switch (current().type) {
        case ADD:
            ret = OP_ADD;
            break;
        case MINUS:
            ret = OP_MINUS;
            break;
        case NOT:
            ret = OP_NOT;
            break;
        default:
            ret = OP_EMPTY;
            break;
    }
    buffer->log(step());

    buffer->log(std::make_shared<ParserLog>(line(), "UnaryOp"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseUnaryExp() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (match({IDENT, LEFT_BRACE})) {
        auto ident = current().value;
        ret->log(step());
        ret->log(step());
        auto params = std::shared_ptr<FuncRParamsNode>();

        if (auto result = tryParse<FuncRParamsNode>(&Parser::parseFuncRParams, ret); result) {
            params = result;
        }
        if (check(RIGHT_BRACE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "unary exp ident() brace missing", buaa::ERROR_MISSING_BRACE));
        }

        ret->node = std::make_shared<UnaryExpNode>(sline, ident, params);
    } else if (check(ADD) || check(MINUS) || check(NOT)) {
        auto op = parseUnaryOp(ret);
        auto exp = tryParse<UnaryExpNode>(&Parser::parseUnaryExp, ret);;

        ret->node = std::make_shared<UnaryExpNode>(sline, op, exp);
    } else if (auto primary = tryParse<PrimaryExpNode>(&Parser::parsePrimary, ret); primary) {
        ret->node = std::make_shared<UnaryExpNode>(sline, primary);
    }

    ret->log(std::make_shared<ParserLog>(line(), "UnaryExp"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseFuncRParams() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    auto exps = std::vector<std::shared_ptr<ExpNode>>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret); exp) {
        exps.push_back(exp);
    } else {
        return ret; // abort
    }
    while (check(COMMA)) {
        ret->log(step());
        if (auto exp = tryParse<ExpNode>(&Parser::parseExp, false, ret); exp) {
            exps.push_back(exp);
        } else {
            return ret; // abort
        }
    }

    ret->node = std::make_shared<FuncRParamsNode>(sline, exps);

    ret->log(std::make_shared<ParserLog>(line(), "FuncRParams"));

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseAddExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseMulExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "AddExp"));
    } else {
        return ret; // abort
    }

    while (check(ADD) || check(MINUS)) {
        auto sline = line();
        auto op = check(ADD) ? OP_ADD : OP_MINUS;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseMulExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "AddExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseMulExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseUnaryExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "MulExp"));
    } else {
        return ret; // abort
    }

    while (check(MULTIPLY) || check(DIVIDE) || check(MOD)) {
        auto sline = line();
        auto op = check(MULTIPLY) ? OP_MUL : check(DIVIDE) ? OP_DIV : OP_MOD;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseUnaryExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "MulExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseRelExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseAddExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "RelExp"));
    } else {
        return ret; // abort
    }

    while (check(GREATER) || check(GREATER_EQUAL) || check(LESSER) || check(LESSER_EQUAL)) {
        auto sline = line();
        auto op = check(GREATER) ? OP_GT : check(GREATER_EQUAL) ? OP_GE : check(LESSER) ? OP_LT : OP_LE;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseAddExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "RelExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseEqExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseRelExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "EqExp"));
    } else {
        return ret; // abort
    }

    while (check(EQUAL) || check(NOT_EQUAL)) {
        auto sline = line();
        auto op = check(EQUAL) ? OP_EQ : OP_NEQ;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseRelExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "EqExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseLAndExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseEqExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "LAndExp"));
    } else {
        return ret; // abort
    }

    while (check(AND)) {
        auto sline = line();
        auto op = OP_AND;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseEqExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "LAndExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseLOrExp() {
    auto ret = std::make_shared<ParseResult>();

    auto node = std::shared_ptr<ExpNode>();
    if (auto exp = tryParse<ExpNode>(&Parser::parseLAndExp, ret); exp) {
        node = exp;
        ret->log(std::make_shared<ParserLog>(line(), "LOrExp"));
    } else {
        return ret; // abort
    }

    while (check(OR)) {
        auto sline = line();
        auto op = OP_OR;
        ret->log(step());
        auto exp = tryParse<ExpNode>(&Parser::parseLAndExp, ret);
        node = std::make_shared<BinaryExpNode>(sline, op, node, exp);
        ret->log(std::make_shared<ParserLog>(line(), "LOrExp"));
    }

    ret->node = node;

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseDecl() {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    auto is_const = false;
    if (check(CONST)) {
        is_const = true;
        ret->log(step());
    }

    auto type = token2Type(current());
    if (!type) {
        return ret; // abort
    }
    ret->log(step());

    auto defs = std::vector<std::shared_ptr<DefNode>>();
    if (auto def = tryParse<DefNode>(&Parser::parseDef, is_const, ret); def) {
        defs.push_back(def);
    } else {
        return ret; // abort
    }
    while (check(COMMA)) {
        ret->log(step());
        if (auto def = tryParse<DefNode>(&Parser::parseDef, is_const, ret); def) {
            defs.push_back(def);
        } else {
            return ret; // abort
        }
    }

    if (check(SEMICOLON)) {
        ret->log(step());
    } else {
        ret->logError(std::make_shared<ErrorLog>(last().line, "decl no semi", buaa::ERROR_MISSING_SEMICOLON));
    }

    if (is_const) {
        ret->log(std::make_shared<ParserLog>(line(), "ConstDecl"));
    } else {
        ret->log(std::make_shared<ParserLog>(line(), "VarDecl"));
    }

    ret->node = std::make_shared<DeclNode>(sline, type, is_const, defs);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseDef(bool is_const) {
    auto ret = std::make_shared<ParseResult>();

    if (!check(IDENT)) {
        return ret; // abort
    }
    auto sline = line();
    auto ident = current().value;
    ret->log(step());
    auto array_exp = std::shared_ptr<ExpNode>();
    if (check(LEFT_SQUARE)) {
        ret->log(step());
        array_exp = tryParse<ExpNode>(&Parser::parseExp, true, ret);
        if (check(RIGHT_SQUARE)) {
            ret->log(step());
        } else {
            ret->logError(std::make_shared<ErrorLog>(last().line, "def no square", buaa::ERROR_MISSING_SQUARE));
        }
    }

    auto init_val = std::shared_ptr<InitValNode>();
    if (is_const) {
        if (!check(ASSIGN)) {
            return ret; // abort
        }
        ret->log(step());
        init_val = tryParse<InitValNode>(&Parser::parseInitVal, is_const, ret);
    } else {
        if (check(ASSIGN)) {
            ret->log(step());
            init_val = tryParse<InitValNode>(&Parser::parseInitVal, is_const, ret);
        }
    }

    if (is_const) {
        ret->log(std::make_shared<ParserLog>(line(), "ConstDef"));
    } else {
        ret->log(std::make_shared<ParserLog>(line(), "VarDef"));
    }

    ret->node = std::make_shared<DefNode>(sline, ident, init_val, is_const, array_exp);

    return ret;
}

std::shared_ptr<ParseResult> Parser::parseInitVal(bool is_const) {
    auto ret = std::make_shared<ParseResult>();

    auto sline = line();
    if (check(LEFT_BRAKET)) {
        ret->log(step());
        auto init_val_set = std::vector<std::shared_ptr<ExpNode>>();
        if (!check(RIGHT_BRAKET)) {
            if (auto exp = tryParse<ExpNode>(&Parser::parseExp, is_const, ret); exp) {
                init_val_set.push_back(exp);
            } else {
                return ret; // abort
            }
            while (check(COMMA)) {
                ret->log(step());
                if (auto exp = tryParse<ExpNode>(&Parser::parseExp, is_const, ret); exp) {
                    init_val_set.push_back(exp);
                } else {
                    return ret; // abort
                }
            }
            if (check(RIGHT_BRAKET)) {
                ret->log(step());
            }
        } else {
            ret->log(step());
        }

        ret->node = std::make_shared<InitValNode>(sline, init_val_set, is_const);
    } else if (check(STRING)) {
        auto str = current().value;
        ret->log(step());

        ret->node = std::make_shared<InitValNode>(sline, str, is_const);
    } else if (auto exp = tryParse<ExpNode>(&Parser::parseExp, is_const, ret); exp) {
        ret->node = std::make_shared<InitValNode>(sline, exp, is_const);
    }

    if (is_const) {
        ret->log(std::make_shared<ParserLog>(line(), "ConstInitVal"));
    } else {
        ret->log(std::make_shared<ParserLog>(line(), "InitVal"));
    }

    return ret;
}



}

}