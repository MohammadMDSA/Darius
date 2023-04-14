
import pathlib
import sys

def main():
    if len(sys.argv) < 5:
        print("GenerateBehaviour <Behaviour Folder Path (absolute)> <Behaviour Class Name> <Behaviour Display Name> <Class Namespace>")
        return

    destination_directory = pathlib.Path(sys.argv[1]).resolve()
    class_name = sys.argv[2]
    display_name = sys.argv[3]
    namespace = sys.argv[4]

    generated_class_footer = "_".join(namespace.split("::")) + "_" + class_name + "_GENERATED"
    generated_file_footer = "File_" + class_name + "_GENERATED"

    current_path = pathlib.Path(__file__).parent.resolve()

    cpp = ''
    hpp = ''

    with open(current_path.joinpath("File Templates/Behaviour Component/TemplateBehaviour.cpp")) as f:
        cpp = f.read();
    
    with open(current_path.joinpath("File Templates/Behaviour Component/TemplateBehaviour.hpp")) as f:
        hpp = f.read();

    cpp = cpp.replace("%%NAMESPACE%%", namespace);
    hpp = hpp.replace("%%NAMESPACE%%", namespace);

    cpp = cpp.replace("%%CLASS_NAME%%", class_name);
    hpp = hpp.replace("%%CLASS_NAME%%", class_name);

    cpp = cpp.replace("%%DISPLAY_NAME%%", display_name);
    hpp = hpp.replace("%%DISPLAY_NAME%%", display_name);

    hpp = hpp.replace("%%FILE_GENERATED%%", generated_file_footer);
    hpp = hpp.replace("%%CLASS_FOOTER_GENERATED%%", generated_class_footer);

    destination_directory.mkdir(parents=True, exist_ok=True)

    cpp_path = destination_directory.joinpath(class_name + ".cpp")
    with open(cpp_path, "w") as f:
        f.write(cpp)

    hpp_path = destination_directory.joinpath(class_name + ".hpp")
    with open(hpp_path, "w") as f:
        f.write(hpp)

    print(str(cpp_path.name) + "\n" + str(hpp_path.name))

main()
    