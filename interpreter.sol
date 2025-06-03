// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract MPCExpressionEvaluator {
    
    // Enums for node types
    enum NodeType { VARIABLE, CONSTANT, OPERATOR, FUNCTION }
    enum OperatorType { ADD, SUB, MUL, DIV }
    enum FunctionType { MAX, MIN, EQUAL, GREATER_THAN, IFELSE, ABSOLUTE }
    
    // Struct to represent expression nodes
    struct ExprNode {
        NodeType nodeType;
        int256 value; // Used for constants and variable indices (0=a, 1=b, 2=c, 3=d)
        OperatorType opType;
        FunctionType funcType;
        uint256 leftChild;  // Index in nodes array
        uint256 rightChild; // Index in nodes array
        uint256[] args;     // For function arguments
        bool isValid;
    }
    
    // Storage for expression tree nodes
    ExprNode[] private nodes;
    mapping(address => ExprNode[]) private userNodes;
    
    // Events
    event ExpressionEvaluated(address indexed user, int256 result);
    event ExpressionParsed(address indexed user, uint256 rootNodeIndex);
    
    // Mathematical operations without using standard operators
    function absolute(int256 a) public pure returns (int256) {
        int256 mask = a >> 255; // Get sign bit (all 1s if negative, all 0s if positive)
        return (a + mask) ^ mask;
    }
    
    function subtract(int256 a, int256 b) public pure returns (int256) {
        return a - b; // Using standard subtraction as in original
    }
    
    function multiply(int256 a, int256 b) public pure returns (int256) {
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
    
    function divideUnsigned(uint256 numerator, uint256 denominator) public pure returns (uint256) {
        if (denominator == 0) return 0;
        if (numerator < denominator) return 0;
        
        uint256 quotient = 0;
        uint256 remainder = numerator;
        
        // Find the highest bit position
        uint256 shift = 0;
        uint256 tempDenom = denominator;
        while (tempDenom <= numerator && shift < 256) {
            tempDenom <<= 1;
            shift++;
        }
        if (shift > 0) shift--;
        
        // Perform bitwise long division
        for (uint256 i = 0; i <= shift; i++) {
            uint256 shiftedDivisor = denominator << (shift - i);
            if (remainder >= shiftedDivisor) {
                remainder -= shiftedDivisor;
                quotient |= (1 << (shift - i));
            }
        }
        
        return quotient;
    }
    
    function divideSigned(int256 a, int256 b) public pure returns (int256) {
        if (b == 0) return 0;
        
        uint256 ua = uint256(absolute(a));
        uint256 ub = uint256(absolute(b));
        uint256 result = divideUnsigned(ua, ub);
        
        bool isNegative = (a < 0) != (b < 0);
        return isNegative ? -int256(result) : int256(result);
    }
    
    function greaterThan(int256 a, int256 b) public pure returns (bool) {
        int256 diff = subtract(a, b);
        return diff > 0;
    }
    
    function equal(int256 a, int256 b) public pure returns (bool) {
        return subtract(a, b) == 0;
    }
    
    function max(int256 a, int256 b) public pure returns (int256) {
        return greaterThan(a, b) ? a : b;
    }
    
    function min(int256 a, int256 b) public pure returns (int256) {
        return greaterThan(b, a) ? a : b;
    }
    
    function ifElse(int256 trueVal, int256 falseVal, bool condition) public pure returns (int256) {
        return condition ? trueVal : falseVal;
    }
    
    // Helper function to create nodes
    function createConstantNode(int256 value) private returns (uint256) {
        ExprNode memory node = ExprNode({
            nodeType: NodeType.CONSTANT,
            value: value,
            opType: OperatorType.ADD, // Default value
            funcType: FunctionType.MAX, // Default value
            leftChild: 0,
            rightChild: 0,
            args: new uint256[](0),
            isValid: true
        });
        
        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }
    
    function createVariableNode(uint256 varIndex) private returns (uint256) {
        require(varIndex < 4, "Variable index must be 0-3 (a-d)");
        
        ExprNode memory node = ExprNode({
            nodeType: NodeType.VARIABLE,
            value: int256(varIndex),
            opType: OperatorType.ADD, // Default value
            funcType: FunctionType.MAX, // Default value
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
            funcType: FunctionType.MAX, // Default value
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
            opType: OperatorType.ADD, // Default value
            funcType: func,
            leftChild: 0,
            rightChild: 0,
            args: argIndices,
            isValid: true
        });
        
        userNodes[msg.sender].push(node);
        return userNodes[msg.sender].length - 1;
    }
    
    // Simplified expression builder functions
    // Users will need to build expressions programmatically
    
    function buildSimpleExpression(
        string memory operation,
        int256 leftOperand,
        int256 rightOperand,
        bool leftIsVariable,
        bool rightIsVariable
    ) external returns (uint256) {
        // Clear previous user nodes
        delete userNodes[msg.sender];
        
        uint256 leftNode;
        uint256 rightNode;
        
        if (leftIsVariable) {
            require(leftOperand >= 0 && leftOperand < 4, "Variable index must be 0-3");
            leftNode = createVariableNode(uint256(leftOperand));
        } else {
            leftNode = createConstantNode(leftOperand);
        }
        
        if (rightIsVariable) {
            require(rightOperand >= 0 && rightOperand < 4, "Variable index must be 0-3");
            rightNode = createVariableNode(uint256(rightOperand));
        } else {
            rightNode = createConstantNode(rightOperand);
        }
        
        OperatorType op;
        if (keccak256(bytes(operation)) == keccak256(bytes("add"))) {
            op = OperatorType.ADD;
        } else if (keccak256(bytes(operation)) == keccak256(bytes("sub"))) {
            op = OperatorType.SUB;
        } else if (keccak256(bytes(operation)) == keccak256(bytes("mul"))) {
            op = OperatorType.MUL;
        } else if (keccak256(bytes(operation)) == keccak256(bytes("div"))) {
            op = OperatorType.DIV;
        } else {
            revert("Invalid operation");
        }
        
        uint256 rootNode = createOperatorNode(op, leftNode, rightNode);
        emit ExpressionParsed(msg.sender, rootNode);
        return rootNode;
    }
    
    function buildFunctionExpression(
        string memory functionName,
        uint256[] memory argValues,
        bool[] memory argIsVariable
    ) external returns (uint256) {
        require(argValues.length == argIsVariable.length, "Arguments length mismatch");
        
        // Clear previous user nodes
        delete userNodes[msg.sender];
        
        FunctionType func;
        uint256 expectedArgs;
        
        if (keccak256(bytes(functionName)) == keccak256(bytes("max"))) {
            func = FunctionType.MAX;
            expectedArgs = 2;
        } else if (keccak256(bytes(functionName)) == keccak256(bytes("min"))) {
            func = FunctionType.MIN;
            expectedArgs = 2;
        } else if (keccak256(bytes(functionName)) == keccak256(bytes("equal"))) {
            func = FunctionType.EQUAL;
            expectedArgs = 2;
        } else if (keccak256(bytes(functionName)) == keccak256(bytes("greater_than"))) {
            func = FunctionType.GREATER_THAN;
            expectedArgs = 2;
        } else if (keccak256(bytes(functionName)) == keccak256(bytes("ifelse"))) {
            func = FunctionType.IFELSE;
            expectedArgs = 3;
        } else if (keccak256(bytes(functionName)) == keccak256(bytes("absolute"))) {
            func = FunctionType.ABSOLUTE;
            expectedArgs = 1;
        } else {
            revert("Invalid function name");
        }
        
        require(argValues.length == expectedArgs, "Invalid number of arguments");
        
        uint256[] memory argNodes = new uint256[](argValues.length);
        for (uint256 i = 0; i < argValues.length; i++) {
            if (argIsVariable[i]) {
                require(argValues[i] < 4, "Variable index must be 0-3");
                argNodes[i] = createVariableNode(argValues[i]);
            } else {
                argNodes[i] = createConstantNode(int256(argValues[i]));
            }
        }
        
        uint256 rootNode = createFunctionNode(func, argNodes);
        emit ExpressionParsed(msg.sender, rootNode);
        return rootNode;
    }
    
    function evaluate(uint256 nodeIndex, int256[4] memory variables) public returns (int256) {
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
                return equal(args[0], args[1]) ? int256(1) : int256(0);
            } else if (node.funcType == FunctionType.GREATER_THAN) {
                return greaterThan(args[0], args[1]) ? int256(1) : int256(0);
            } else if (node.funcType == FunctionType.IFELSE) {
                return ifElse(args[0], args[1], args[2] != 0);
            } else if (node.funcType == FunctionType.ABSOLUTE) {
                return absolute(args[0]);
            }
        }
        
        revert("Invalid node type");
    }
    
    function evaluateExpression(uint256 rootNodeIndex, int256 a, int256 b, int256 c, int256 d) external returns (int256) {
        int256[4] memory variables = [a, b, c, d];
        int256 result = evaluate(rootNodeIndex, variables);
        emit ExpressionEvaluated(msg.sender, result);
        return result;
    }
    
    // Utility functions
    function clearUserNodes() external {
        delete userNodes[msg.sender];
    }
    
    function getUserNodeCount() external view returns (uint256) {
        return userNodes[msg.sender].length;
    }
    
    function getNodeInfo(uint256 nodeIndex) external view returns (
        NodeType nodeType,
        int256 value,
        OperatorType opType,
        FunctionType funcType,
        uint256 leftChild,
        uint256 rightChild,
        uint256[] memory args
    ) {
        require(nodeIndex < userNodes[msg.sender].length, "Invalid node index");
        ExprNode storage node = userNodes[msg.sender][nodeIndex];
        
        return (
            node.nodeType,
            node.value,
            node.opType,
            node.funcType,
            node.leftChild,
            node.rightChild,
            node.args
        );
    }
}
