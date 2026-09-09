SRC = src/TokenType.java src/Token.java src/Lexer.java \
      src/AST.java src/Parser.java src/TypeChecker.java \
      src/Interpreter.java src/Main.java

all:
	javac -d out $(SRC)

run:
	java -cp out Main $(FILE)

ast:
	java -cp out Main $(FILE) --dump-ast

clean:
	rm -rf out/*.class

test:
	java -cp out Main examples/valid/program1.ss
	java -cp out Main examples/valid/program2.ss
	java -cp out Main examples/valid/program3.ss
	-java -cp out Main examples/invalid/error1.ss
	-java -cp out Main examples/invalid/error2.ss
	-java -cp out Main examples/invalid/error3.ss
	-java -cp out Main examples/invalid/error4.ss
	-java -cp out Main examples/invalid/error5.ss
	-java -cp out Main examples/invalid/error_type.ss
