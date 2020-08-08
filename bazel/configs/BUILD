# Build configuration values.
package(default_visibility = ["//visibility:public"])

# Config settings are used by build rules to selectively choose includes, compile flags, etc.
config_setting(name = "hitl32", define_values = {"flight" : "0", "target" : "teensy32", "gsw" : "0"})
config_setting(name = "hitl35", define_values = {"flight" : "0", "target" : "teensy35", "gsw" : "0"})
config_setting(name = "hitl36", define_values = {"flight" : "0", "target" : "teensy36", "gsw" : "0"})

config_setting(name = "hootl", define_values = {"flight" : "0", "target" : "desktop", "gsw" : "0"})
config_setting(name = "gsw", define_values = {"flight" : "1", "target" : "desktop", "gsw" : "1", "timing" : "realtime"})
config_setting(name = "flight", define_values = {"flight" : "1", "target" : "teensy36", "gsw" : "0", "timing" : "realtime"})

config_setting(name = "adcs", define_values = {"flight" : "1", "target" : "teensy35", "gsw" : "0", "timing" : "realtime"})

config_setting(name = "leader", define_values = {"satellite" : "leader"})
config_setting(name = "follower", define_values = {"satellite" : "follower"})

config_setting(name = "realtime", define_values = {"timing" : "realtime"})
config_setting(name = "accelerated", define_values = {"timing" : "accelerated"})

config_setting(name = "unit_test", define_values = {"unit_test" : "1"})
