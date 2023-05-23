
import pathlib
import sys

def main():
    if len(sys.argv) < 7:
        print("GenerateResource <Resource Folder Path (relative to project root)> <Resource Class Name> <Resource Name> <Class Namespace> <Class Namespace Macro> <Supported Extensions... (\".foo\", \".bar\", ...)> ")
        return

    path = pathlib.Path(sys.argv[1]).resolve()
    class_name = sys.argv[2]
    resource_name = sys.argv[3]
    namespace = sys.argv[4]
    namespace_key = sys.argv[5]
    exts = sys.argv[6:]

    current_path = pathlib.Path(__file__).parent.resolve()
    project_path = current_path.parent.resolve()
    
    generated_class_footer = "_".join(namespace.split("::")) + "_" + class_name + "_GENERATED"
    generated_file_footer = "File_" + class_name + "_GENERATED"

    cpp = ''
    hpp = ''

    with open(current_path.joinpath("File Templates/Resource/ResourceTemplate.cpp")) as f:
        cpp = f.read();
    
    with open(current_path.joinpath("File Templates/Resource/ResourceTemplate.hpp")) as f:
        hpp = f.read();

    cpp = cpp.replace("%%NAMESPACE_KEY%%", namespace_key);
    hpp = hpp.replace("%%NAMESPACE_KEY%%", namespace_key);

    cpp = cpp.replace("%%NAMESPACE%%", namespace);
    hpp = hpp.replace("%%NAMESPACE%%", namespace);

    cpp = cpp.replace("%%CLASS_NAME%%", class_name);
    hpp = hpp.replace("%%CLASS_NAME%%", class_name);

    cpp = cpp.replace("%%RESOURCE_NAME%%", '"' + resource_name + '"');
    hpp = hpp.replace("%%RESOURCE_NAME%%", '"' + resource_name + '"');

    ext = '"' + '", "'.join(exts) + '"'

    cpp = cpp.replace("%%SUPPORTED_EXT%%", ext);
    hpp = hpp.replace("%%SUPPORTED_EXT%%", ext);
    
    hpp = hpp.replace("%%FILE_GENERATED%%", generated_file_footer);
    hpp = hpp.replace("%%CLASS_FOOTER_GENERATED%%", generated_class_footer);

    destination_directory = project_path.joinpath(path)
    destination_directory.mkdir(parents=True, exist_ok=True)

    cpp_path = destination_directory.joinpath(class_name + ".cpp")
    with open(cpp_path, "w") as f:
        f.write(cpp)

    hpp_path = destination_directory.joinpath(class_name + ".hpp")
    with open(hpp_path, "w") as f:
        f.write(hpp)

    print(str(cpp_path.name) + "\n" + str(hpp_path.name))

main()
    