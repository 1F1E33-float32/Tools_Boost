add_rules("mode.release", "mode.debug")
add_rules("plugin.compile_commands.autoupdate", {
    outputdir = "$(builddir)"
})
add_requires("nanobind", "zlib")

target("tools_boost")
do
    set_prefixdir("$(pythondir)/tools_boost")
    add_rules("python.module")
    add_files("src/*.cpp")
    add_packages("nanobind", "zlib")

    set_languages("c++17")
end

target("tools_boost_pkg")
do
    set_kind("phony")

    add_deps("tools_boost")

    add_installfiles("src/__init__.py", {
        prefixdir = "$(pythondir)/tools_boost"
    })
    add_installfiles("src/__init__.pyi", {
        prefixdir = "$(pythondir)/tools_boost"
    })

    add_installfiles("$(builddir)/$(plat)/$(arch)/$(mode)/tools_boost*.pyd", {
        prefixdir = "$(pythondir)/tools_boost"
    })
    add_installfiles("$(builddir)/$(plat)/$(arch)/$(mode)/tools_boost*.so", {
        prefixdir = "$(pythondir)/tools_boost"
    })
end
