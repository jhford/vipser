CC=clang
CFLAGS=-g -Wall -Werror `pkg-config --cflags vips glib-2.0`
LDFLAGS=`pkg-config --libs vips glib-2.0`

vipser: vipser.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
