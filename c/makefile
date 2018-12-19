var1=linux      #one variable value is "linux      "

include include.mk

$(warning level:$(MAKELEVEL))

target=test

all:$(target)

include $(target).d
CFLAGS += -g

$(target):$(target).o
	gcc $(CFLAGS) -o $@ $<

%.o:%.c
	gcc $(CFLAGS) -o $@ -c $< 

%.d:%.c
	gcc -MM $< > $@

clean:
	rm -rf $(target) *.o *.d


