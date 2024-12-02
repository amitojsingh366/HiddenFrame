.PHONY: all
all: build build/sqlite.o build/utils.o build/image_class.o build/authorization.o build/db_operations.o build/snowflake.o build/api.o
	g++ $(ObjectFiles) -o bin/HiddenFrame ${LinkerOptions} ${CompilationWarnings}

.PHONY: run
run: all
	$(EXECUTE)

build/sqlite.o: include/sqlite/sqlite3.c
	g++ -x c -std=c99 -c $< -o $@ ${IncludePaths} ${CompilationWarnings}

build/utils.o: src/utils.cpp include/hiddenframe_headers.h include/utils.h
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings}

build/image_class.o: src/image_class.cpp include/hiddenframe_headers.h include/stb_image_write.h include/stb_image.h include/utils.h
	g++ -c $< -o $@ ${CompilationOptions}

build/authorization.o: src/authorization.cpp include/jwt-cpp/* include/picojson/*  include/crow/* include/authorization.h
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${LinkerOptions}


build/db_operations.o: src/db_operations.cpp include/hiddenframe_headers.h build/sqlite.o
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${LinkerOptions}

build/snowflake.o: src/snowflake.cpp include/snowflake.h
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${LinkerOptions}

build/api.o: src/api.cpp include/hiddenframe_headers.h include/crow.h include/crow/* include/authorization.h include/snowflake.h
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${LinkerOptions}

build/test/test_image_io.o: test/test_image_io.cpp include/hiddenframe_headers.h
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings}

build/test/test_user_database.o: test/test_user_database.cpp include/hiddenframe_headers.h include/sqlite/*
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${IncludePaths}

build/test/main.o: test/main.cpp
	g++ -c $< -o $@ ${CompilationOptions} ${CompilationWarnings} ${IncludePaths}

.PHONY: test
test: build build/utils.o build/image_class.o build/authorization.o build/db_operations.o build/api.o build/test/test_image_io.o build/test/test_user_database.o build/test/main.o
	g++ build/utils.o build/image_class.o build/authorization.o build/db_operations.o build/sqlite.o ${TestObjectFiles} -o bin/test/HiddenFrame_Test ${LinkerOptions} ${CompilationWarnings}

.PHONY: run-test
run-test: test
	$(CLEAN_DIR) bin/test/database 
	$(MKDIR) bin/test/database 
	$(RUNTEST)	

macos: all

linux: all

windows: all

.PHONY: build
build:
	$(MKDIR) build
	$(MKDIR) build/test
	$(MKDIR) static
	$(MKDIR) bin
	$(MKDIR) bin/test
	$(MKDIR) bin/test/database
	$(MKDIR) database

.PHONY: clean
clean:
	$(CLEAN_DIR) build
	$(CLEAN_DIR) bin
	$(CLEAN_DIR) static
#	$(CLEAN_DIR) database

IncludePaths  ?= -I include/ 
CompilationOptions= -ggdb -std=c++17 -DCROW_USE_BOOST ${IncludePaths}
LinkerOptions= -lcrypto
CompilationWarnings=-Wall -Wextra
ObjectFiles= build/utils.o build/image_class.o build/authorization.o build/db_operations.o build/snowflake.o build/sqlite.o build/api.o
TestObjectFiles= build/test/test_image_io.o build/test/test_user_database.o build/test/main.o

ifeq ($(OS),Windows_NT)
	OS := windows
else
	OS := $(shell uname)
endif

ifeq ($(OS),Linux)
	RM := rm -rf
	MKDIR := mkdir -p
	CLEAN_DIR := rm -rf static/*
	EXECUTE := JWT_SECRET=secret ./bin/HiddenFrame
	RUNTEST := BOOST_TEST_LOG_LEVEL=message ./bin/test/HiddenFrame_Test
	IncludePaths += -I include/ -I /usr/local/include
else ifeq ($(OS),Darwin)
	RM := rm -rf
	MKDIR := mkdir -p
	CLEAN_DIR := rm -rf static/*
	EXECUTE := JWT_SECRET=secret ./bin/HiddenFrame
	RUNTEST := BOOST_TEST_LOG_LEVEL=message ./bin/test/HiddenFrame_Test
	IncludePaths += -I include/ -I /opt/homebrew/include
	LinkerOptions += -L /opt/homebrew/opt/openssl/lib
else
	RM := rmdir /s /q 
	MKDIR := @powershell -Command "New-Item -ItemType Directory -Force -Path " 
	CLEAN_DIR := @powershell -Command "Remove-Item -Recurse -Force"
	EXECUTE := powershell -Command "$$env:JWT_SECRET='secret'; & ./bin/HiddenFrame.exe"
	RUNTEST := powershell -Command "$$env:BOOST_TEST_LOG_LEVEL='message'; & ./bin/test/HiddenFrame_Test.exe"
	IncludePaths += -I include/ -I include/boost -I include/openssl -I "C:/Program Files/OpenSSL/include"
	LinkerOptions += -L "C:/Program Files/OpenSSL/lib" -lws2_32 -lmswsock
endif
