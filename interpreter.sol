// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract MPCExpressionEvaluator {

    enum TokenType { VARIABLE, NUMBER, OPERATOR, LPAREN, RPAREN, FUNCTION, COMMA, EOF }
    enum NodeType { VARIABLE, CONSTANT, OPERATOR, FUNCTION }
    enum OperatorType { ADD, SUB, MUL, DIV }
    enum FunctionType { MAX, MIN, EQUAL, GREATER_THAN, IFELSE, ABSOLUTE }

    struct Token {
        TokenType tokenType;
        string value;
        int256 numValue;
    }

    struct ExprNode {
        NodeType nodeType;
        int256 value;
        OperatorType opType;
        FunctionType funcType;
        uint256 leftChild;
        uint256 rightChild;
        uint256[] args;
        bool isValid;
    }

    mapping(address => ExprNode[]) private userNodes;
    mapping(address => Token[]) private userTokens;
    mapping(address => uint256) private parsePosition;

    event ExpressionEvaluated(address indexed user, string expression, int256 result);
    event ExpressionParsed(address indexed user, string expression, uint256 rootNodeIndex);

    function absolute(int256 a) private pure returns (int256) {
        int256 mask = a >> 255;
        return (a + mask) ^ mask;
    }

    function subtract(int256 a, int256 b) private pure returns (int256) {
        return a - b;
    }

    function multiply(int256 a, int256 b) private pure returns (int256) {
        if (a == 0 || b == 0) return 0;

        bool isNegative = (a < 0) != (b < 0);
        uint256 ua = uint256(absolute(a));
        uint256 ub = uint256(absolute(b));

        uint256 result = 0;
        for (uint256 i = 0; i < 256 && ub > 0; i++) {
            if (ub & 1 == 1) {
                result += ua;
            }
            ua <<= 1;
            ub >>= 1;
        }

        return isNegative ? -int256(result) : int256(result);
    }

    function divideUnsigned(uint256 numerator, uint256 denominator) private pure returns (uint256) {
        if (denominator == 0) return 0;
        if (numerator < denominator) return 0;

        uint256 quotient = 0;
        uint256 remainder = numerator;

        uint256 shift = 0;
        while ((denominator << shift) <= numerator && (denominator << shift) != 0 && shift < 255) {
            shift++;
        }
        if (shift > 0 && (denominator << shift) > numerator) {
             shift--;
        } else if (shift == 255 && (denominator << shift) <= numerator) {
        }

        for (uint256 i = 0; i <= shift; i++) {
            uint256 shiftedDivisor = denominator << (shift - i);

            if (remainder >= shiftedDivisor) {
                remainder -= shiftedDivisor;
                quotient |= (1 << (shift - i));
            }
        }

        return quotient;
    }

    function divideSigned(int256 a, int256 b) private pure returns (int256) {
        if (b == 0) return 0;

        uint256 ua = uint256(absolute(a));
        uint256 ub = uint256(absolute(b));
        uint256 resultUnsigned = divideUnsigned(ua, ub);

        bool isNegative = (a < 0) != (b < 0);
        return isNegative ? -int256(resultUnsigned) : int256(resultUnsigned);
    }

    function greaterThan(int256 a, int256 b) private pure returns (int256) {
        int256 diff = subtract(a, b);
        return diff > 0 ? int256(1) : int256(0);
    }

    function equal(int256 a, int256 b) private pure returns (int256) {
        int256 diff = subtract(a, b);
        return diff == 0 ? int256(1) : int256(0);
    }

    function ifElse(int256 trueVal, int256 falseVal, int256 conditionVal) private pure returns (int256) {
        int256 mask = conditionVal == int256(1) ? int256(-1) : int256(0);
        return (trueVal & mask) | (falseVal & (~mask));
    }

    function max(int256 a, int256 b) private pure returns (int256) {
        int256 cond = greaterThan(a, b);
        return ifElse(a, b, cond);
    }

    function min(int256 a, int256 b) private pure returns (int256) {
        int256 cond = greaterThan(b, a);
        return ifElse(a, b, cond);
    }

    function isAlpha(bytes1 char) private pure returns (bool) {
        return (char >= 0x41 && char <= 0x5A) || (char >= 0x61 && char <= 0x7A);
    }

    function isDigit(bytes1 char) private pure returns (bool) {
        return char >= 0x30 && char <= 0x39;
    }

    function isSpace(bytes1 char) private pure returns (bool) {
        return char == 0x20 || char == 0x09 || char == 0x0A || char == 0x0D;
    }

    function stringToInt(string memory str) private pure returns (int256) {
        bytes memory b = bytes(str);
        int256 result = 0;
        bool negative = false;
        uint256 start = 0;

        if (b.length > 0 && b[0] == 0x2D) {
            negative = true;
            start = 1;
        }

        for (uint256 i = start; i < b.length; i++) {
            if (isDigit(b[i])) {
                result = result * 10 + int256(uint256(uint8(b[i]) - 48));
            }
        }

        return negative ? -result : result;
    }

    function stringsEqual(string memory a, string memory b) private pure returns (bool) {
        return keccak256(bytes(a)) == keccak256(bytes(b));
    }

    function isFunction(string memory word) private pure returns (bool) {
        return stringsEqual(word, "max") || stringsEqual(word, "min") ||
               stringsEqual(word, "equal") || stringsEqual(word, "greater_than") ||
               stringsEqual(word, "ifelse") || stringsEqual(word, "absolute");
    }

    function tokenize(string memory expression) private {
        bytes memory expr = bytes(expression);
        delete userTokens[msg.sender];

        uint256 pos = 0;

        while (pos < expr.length) {
            if (isSpace(expr[pos])) {
                pos++;
                continue;
            }

            if (isAlpha(expr[pos])) {
                uint256 start = pos;
                while (pos < expr.length && (isAlpha(expr[pos]) || isDigit(expr[pos]) || expr[pos] == 0x5F)) {
                    pos++;
                }

                string memory word = substring(expression, start, pos);

                Token memory token;
                token.value = word;
                token.tokenType = isFunction(word) ? TokenType.FUNCTION : TokenType.VARIABLE;
                userTokens[msg.sender].push(token);
            }
            else if (isDigit(expr[pos]) || (expr[pos] == 0x2D && pos + 1 < expr.length && isDigit(expr[pos + 1]))) {
                uint256 start = pos;
                if (expr[pos] == 0x2D) pos++;
                while (pos < expr.length && isDigit(expr[pos])) {
                    pos++;
                }

                string memory numStr = substring(expression, start, pos);
                Token memory token;
                token.value = numStr;
                token.numValue = stringToInt(numStr);
                token.tokenType = TokenType.NUMBER;
                userTokens[msg.sender].push(token);
            }
            else {
                Token memory token;

                if (expr[pos] == 0x2B) {
                    token.tokenType = TokenType.OPERATOR;
                    token.value = "+";
                } else if (expr[pos] == 0x2D) {
                    token.tokenType = TokenType.OPERATOR;
                    token.value = "-";
                } else if (expr[pos] == 0x2A) {
                    token.tokenType = TokenType.OPERATOR;
                    token.value = "*";
                } else if (expr[pos] == 0x2F) {
                    token.tokenType = TokenType.OPERATOR;
                    token.value = "/";
                } else if (expr[pos] == 0x28) {
                    token.tokenType = TokenType.LPAREN;
                    token.value = "(";
                } else if (expr[pos] == 0x29) {
                    token.tokenType = TokenType.RPAREN;
                    token.value = ")";
                } else if (expr[pos] == 0x2C) {
                    token.tokenType = TokenType.COMMA;
                    token.value = ",";
                } else {
                    revert("Unexpected character in expression");
                }

                userTokens[msg.sender].push(token);
                pos++;
            }
        }

        Token memory eofToken;
        eofToken.tokenType = TokenType.EOF;
        eofToken.value = "";
        userTokens[msg.sender].push(eofToken);
    }

    function substring(string memory str, uint256 start, uint256 end) private pure returns (string memory) {
        bytes memory strBytes = bytes(str);
        bytes memory result = new bytes(end - start);
        for (uint256 i = start; i < end; i++) {
            result[i - start] = strBytes[i];
        }
        return string(result);
    }

    function createConstantNode(int256 value) private returns (uint256) {
        ExprNode memory node = ExprNode({
            nodeType: NodeType.CONSTANT,
            value: value,
            opType: OperatorType.ADD,
            funcType: FunctionType.MAX,
            leftChild: 0,
            rightChild: 0,
            args: new uint256[](0),
            isValid: true
        });

        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }

    function createVariableNode(uint256 varIndex) private returns (uint256) {
        ExprNode memory node = ExprNode({
            nodeType: NodeType.VARIABLE,
            value: int256(varIndex),
            opType: OperatorType.ADD,
            funcType: FunctionType.MAX,
            leftChild: 0,
            rightChild: 0,
            args: new uint256[](0),
            isValid: true
        });

        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }

    function createOperatorNode(OperatorType op, uint256 left, uint256 right) private returns (uint256) {
        ExprNode memory node = ExprNode({
            nodeType: NodeType.OPERATOR,
            value: 0,
            opType: op,
            funcType: FunctionType.MAX,
            leftChild: left,
            rightChild: right,
            args: new uint256[](0),
            isValid: true
        });

        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }

    function createFunctionNode(FunctionType func, uint256[] memory argIndices) private returns (uint256) {
        ExprNode memory node = ExprNode({
            nodeType: NodeType.FUNCTION,
            value: 0,
            opType: OperatorType.ADD,
            funcType: func,
            leftChild: 0,
            rightChild: 0,
            args: argIndices,
            isValid: true
        });

        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }

    function getCurrentToken() private view returns (Token memory) {
        uint256 pos = parsePosition[msg.sender];
        if (pos < userTokens[msg.sender].length) {
            return userTokens[msg.sender][pos];
        }
        Token memory eofToken;
        eofToken.tokenType = TokenType.EOF;
        return eofToken;
    }

    function advanceToken() private {
        parsePosition[msg.sender]++;
    }

    function parseFactor() private returns (uint256) {
        Token memory token = getCurrentToken();

        if (token.tokenType == TokenType.NUMBER) {
            advanceToken();
            return createConstantNode(token.numValue);
        }

        if (token.tokenType == TokenType.VARIABLE) {
            advanceToken();
            bytes memory varBytes = bytes(token.value);
            require(varBytes.length == 1, "Variables must be single characters");

            uint256 varIndex;
            if (varBytes[0] == 0x61) varIndex = 0;
            else if (varBytes[0] == 0x62) varIndex = 1;
            else if (varBytes[0] == 0x63) varIndex = 2;
            else if (varBytes[0] == 0x64) varIndex = 3;
            else revert("Invalid variable. Use a, b, c, or d");

            return createVariableNode(varIndex);
        }

        if (token.tokenType == TokenType.FUNCTION) {
            return parseFunction();
        }

        if (token.tokenType == TokenType.LPAREN) {
            advanceToken();
            uint256 expr = parseExpression();
            require(getCurrentToken().tokenType == TokenType.RPAREN, "Expected closing parenthesis");
            advanceToken();
            return expr;
        }

        revert("Unexpected token in expression");
    }

    function parseFunction() private returns (uint256) {
        Token memory funcToken = getCurrentToken();
        advanceToken();

        require(getCurrentToken().tokenType == TokenType.LPAREN, "Expected opening parenthesis after function");
        advanceToken();

        uint256[] memory args = new uint256[](3);
        uint256 argc = 0;

        if (getCurrentToken().tokenType != TokenType.RPAREN) {
            args[argc] = parseExpression();
            argc++;

            while (getCurrentToken().tokenType == TokenType.COMMA) {
                advanceToken();
                require(argc < 3, "Too many function arguments");
                args[argc] = parseExpression();
                argc++;
            }
        }

        require(getCurrentToken().tokenType == TokenType.RPAREN, "Expected closing parenthesis");
        advanceToken();

        FunctionType func;
        if (stringsEqual(funcToken.value, "max")) {
            require(argc == 2, "max function requires 2 arguments");
            func = FunctionType.MAX;
        } else if (stringsEqual(funcToken.value, "min")) {
            require(argc == 2, "min function requires 2 arguments");
            func = FunctionType.MIN;
        } else if (stringsEqual(funcToken.value, "equal")) {
            require(argc == 2, "equal function requires 2 arguments");
            func = FunctionType.EQUAL;
        } else if (stringsEqual(funcToken.value, "greater_than")) {
            require(argc == 2, "greater_than function requires 2 arguments");
            func = FunctionType.GREATER_THAN;
        } else if (stringsEqual(funcToken.value, "ifelse")) {
            require(argc == 3, "ifelse function requires 3 arguments");
            func = FunctionType.IFELSE;
        } else if (stringsEqual(funcToken.value, "absolute")) {
            require(argc == 1, "absolute function requires 1 argument");
            func = FunctionType.ABSOLUTE;
        } else {
            revert("Unknown function");
        }

        uint256[] memory actualArgs = new uint256[](argc);
        for (uint256 i = 0; i < argc; i++) {
            actualArgs[i] = args[i];
        }

        return createFunctionNode(func, actualArgs);
    }

    function parseTerm() private returns (uint256) {
        uint256 left = parseFactor();

        while (getCurrentToken().tokenType == TokenType.OPERATOR) {
            Token memory op = getCurrentToken();
            if (stringsEqual(op.value, "*") || stringsEqual(op.value, "/")) {
                advanceToken();
                uint256 right = parseFactor();

                OperatorType opType;
                if (stringsEqual(op.value, "*")) {
                    opType = OperatorType.MUL;
                } else {
                    opType = OperatorType.DIV;
                }

                left = createOperatorNode(opType, left, right);
            } else {
                break;
            }
        }

        return left;
    }

    function parseExpression() private returns (uint256) {
        uint256 left = parseTerm();

        while (getCurrentToken().tokenType == TokenType.OPERATOR) {
            Token memory op = getCurrentToken();
            if (stringsEqual(op.value, "+") || stringsEqual(op.value, "-")) {
                advanceToken();
                uint256 right = parseTerm();

                OperatorType opType;
                if (stringsEqual(op.value, "+")) {
                    opType = OperatorType.ADD;
                } else {
                    opType = OperatorType.SUB;
                }

                left = createOperatorNode(opType, left, right);
            } else {
                break;
            }
        }

        return left;
    }

    function parse(string memory expression) private returns (uint256) {
        delete userNodes[msg.sender];
        delete userTokens[msg.sender];
        parsePosition[msg.sender] = 0;

        tokenize(expression);
        uint256 rootNode = parseExpression();

        require(getCurrentToken().tokenType == TokenType.EOF, "Unexpected tokens at end of expression");

        emit ExpressionParsed(msg.sender, expression, rootNode);
        return rootNode;
    }

    function evaluate(uint256 nodeIndex, int256[4] memory variables) private returns (int256) {
        require(nodeIndex < userNodes[msg.sender].length, "Invalid node index");
        ExprNode storage node = userNodes[msg.sender][nodeIndex];
        require(node.isValid, "Invalid node");

        if (node.nodeType == NodeType.VARIABLE) {
            uint256 varIndex = uint256(node.value);
            require(varIndex < 4, "Invalid variable index");
            return variables[varIndex];
        }

        if (node.nodeType == NodeType.CONSTANT) {
            return node.value;
        }

        if (node.nodeType == NodeType.OPERATOR) {
            int256 leftVal = evaluate(node.leftChild, variables);
            int256 rightVal = evaluate(node.rightChild, variables);

            if (node.opType == OperatorType.ADD) {
                return leftVal + rightVal;
            } else if (node.opType == OperatorType.SUB) {
                return subtract(leftVal, rightVal);
            } else if (node.opType == OperatorType.MUL) {
                return multiply(leftVal, rightVal);
            } else if (node.opType == OperatorType.DIV) {
                require(rightVal != 0, "Division by zero");
                return divideSigned(leftVal, rightVal);
            }
        }

        if (node.nodeType == NodeType.FUNCTION) {
            int256[] memory args = new int256[](node.args.length);
            for (uint256 i = 0; i < node.args.length; i++) {
                args[i] = evaluate(node.args[i], variables);
            }

            if (node.funcType == FunctionType.MAX) {
                return max(args[0], args[1]);
            } else if (node.funcType == FunctionType.MIN) {
                return min(args[0], args[1]);
            } else if (node.funcType == FunctionType.EQUAL) {
                return equal(args[0], args[1]);
            } else if (node.funcType == FunctionType.GREATER_THAN) {
                return greaterThan(args[0], args[1]);
            } else if (node.funcType == FunctionType.IFELSE) {
                return ifElse(args[0], args[1], args[2]);
            } else if (node.funcType == FunctionType.ABSOLUTE) {
                return absolute(args[0]);
            }
        }

        revert("Invalid node type during evaluation");
    }

    function evaluateExpression(string memory expression, int256 a, int256 b, int256 c, int256 d) external returns (int256) {
        uint256 rootNode = parse(expression);
        int256[4] memory variables = [a, b, c, d];
        int256 result = evaluate(rootNode, variables);

        emit ExpressionEvaluated(msg.sender, expression, result);
        return result;
    }
}
