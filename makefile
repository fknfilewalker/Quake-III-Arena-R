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

clean:
ifeq ($(OS), Darwin)
	rm -r _project & rm -r bin
else
	rm -r _project & rm -r bin
endif
