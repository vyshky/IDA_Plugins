#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <hexrays.hpp>

// �������� ������� Hex-Rays
bool init_hexrays_plugin_my() {
    if (!init_hexrays_plugin()) {
        msg("Hex-Rays Decompiler �� ������!\n");
        return false;
    }
    return true;
}

// ������� ��� ��������� ������� Hex-Rays
// TODO:: ����� �������� � ����������� ���� ����� ����� ��������
static long long idaapi my_callback(void* user_data, hexrays_event_t event_id, va_list va) {
    if (event_id == hxe_right_click) {
        vdui_t* vu = va_arg(va, vdui_t*);
        if (vu && vu->cfunc) {
            qstring decompiled_code;
            vu->cfunc->body.print1(&decompiled_code, vu->cfunc);
            msg("����������������� ���:\n%s\n", decompiled_code.c_str());
        }
    }
    return 0;
}

// ������-����� ��� IDA 9.0
struct my_plugin_t : public plugmod_t {
    my_plugin_t() {
        if (init_hexrays_plugin_my()) {
            install_hexrays_callback(my_callback, nullptr);
        }
    }
    ~my_plugin_t() override {
        remove_hexrays_callback(my_callback, nullptr);
    }
    bool idaapi run(size_t) { return true; };
};

//bool idaapi my_plugin_t::run(size_t)
//{
//    msg("Hello, world! (cpp)\n");
//    return true;
//}

// ������� ������������� �������
static plugmod_t* idaapi init() {
    return new my_plugin_t();
}

// ����������� �������
plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,
     PLUGIN_UNL | PLUGIN_MULTI,
    init,
    nullptr,
    nullptr,
    "XXX_PRIMERPLUGINIDA9.0",
    "XXX_primerplugin",
    "XXX_primerPlugin",
    "Ctrl-Shift-G"
};