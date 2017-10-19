ifeq ($(var1),)
  $(warning "var1 is empty")
else ifeq ($(var1),linux)
  $(warning var1 is $(var1))
else
  $(warning var1 is $(var1)end)
endif

$(warning level:$(MAKELEVEL))
