
project "sedit"
    language "C++"
    kind "ConsoleApp"

    vpaths
    {
        ["*"] = "premake5.lua"
    }

    includedirs
    {
        "."
    }

    files
    {
        "premake5.lua",
        "**.h",
        "**.cpp"
    }