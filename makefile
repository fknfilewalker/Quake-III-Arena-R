OS := $(shell uname)

all:
ifeq ($(OS), Darwin)
	@echo "OS" $(OS)
	cmake -H. -B_project -G "Xcode"
else
	@echo "OS" $(OS)
	cmake -H. -B_project -G "Unix Makefiles"
	cd _project && make
endif

debug:
	@echo "OS" $(OS)
	@echo "Generate Debug Makefiles"
	cmake -H. -B_project -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"
	cd _project && make

release:
	@echo "OS" $(OS)
	@echo "Generate Release Makefiles"
	cmake -H. -B_project -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"
	cd _project && make

clean:
ifeq ($(OS), Darwin)
	rm -r _project & rm -r bin
else
	rm -r _project & rm -r bin
endif
