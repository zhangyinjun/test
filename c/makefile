var1=linux      #one variable value is "linux      "

include include.mk

$(warning level:$(MAKELEVEL))

target=test

all:$(target)

include $(target).d

$(target):$(target).o
	gcc -o $@ $<

%.o:%.c
	gcc -o $@ -c $< 

%.d:%.c
	gcc -MM $< > $@

clean:
	rm -rf $(target) *.o *.d


