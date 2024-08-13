# CMakeList.txt: Sandbox �� CMake ��Ŀ���ڴ˴�����Դ���벢����
# ��Ŀ�ض����߼���
#

project (Sandbox)


# ��Դ�������ӵ�����Ŀ�Ŀ�ִ���ļ���
add_executable (Sandbox "main.cpp")

set_property(TARGET Sandbox PROPERTY CXX_STANDARD 17)

target_link_libraries(Sandbox PRIVATE Core)

# TODO: ������Ҫ�������Ӳ��Բ���װĿ�ꡣ