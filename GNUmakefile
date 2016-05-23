projects := jbot jbotd

jbot_cfg := release checked debug

jbot_cflags := -DZLX_STATIC -DHBS_STATIC
jbot_csrc := jbot.c elal_test.c mth_test.c
jbot_ldflags = -static -lhbs$($3_sfx) -lzlx$($3_sfx) $(if $(findstring -windows,$($4_target)),-mconsole -municode,-lpthread)
jbot_ldep = $(call prod_path,zlx,slib,$3,$4) $(call prod_path,hbs,slib,$3,$4)

jbotd_cfg := release checked debug
jbotd_cflags :=
jbotd_csrc := $(jbot_csrc)
jbotd_ldflags = $(filter-out -static,$(jbot_ldflags))
jbotd_ldep = $(call prod_path,zlx,dlib,$3,$4) $(call prod_path,hbs,dlib,$3,$4)

include icobld.mk

