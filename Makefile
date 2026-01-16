BINARY:=utfConverter

LIB_REPO = https://github.com/OmarMesqq/cutilities
LIB_PATH := libs/libcutilities


INCLUDES := -I$(LIB_PATH)
# -L tells it where to look during compile
# -Wl,-rpath tells it where to look during execution
LDFLAGS  := -L$(LIB_PATH) -lcutilities -Wl,-rpath,$(shell pwd)/$(LIB_PATH)
$(LIB_PATH):
	mkdir -p libs
	git clone $(LIB_REPO) $(LIB_PATH)
	$(MAKE) -C $(LIB_PATH) release


debug: $(LIB_PATH)
	gcc -g -Wall -Wextra main.c convutf.c $(INCLUDES) $(LDFLAGS) -o $(BINARY)

release: $(LIB_PATH)
	gcc -O3 -Wall -Wextra main.c convutf.c $(INCLUDES) $(LDFLAGS) -o $(BINARY)

clean:
	rm -rf $(BINARY) generated *.dSYM
