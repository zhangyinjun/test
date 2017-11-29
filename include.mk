ifeq ($(var1),)
  $(info "var1 is empty")
else ifeq ($(var1),linux)
  $(info var1 is $(var1))
else
  $(info var1 is $(var1)end)
endif

$(info level:$(MAKELEVEL))
