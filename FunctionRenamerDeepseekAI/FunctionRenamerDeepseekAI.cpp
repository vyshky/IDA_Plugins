﻿/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2024 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler.
 *      It decompiles the current function and prints it in the message window
 *
 */

#include <hexrays.hpp>
#include <map>  
#include <string> 

 //--------------------------------------------------------------------------
struct plugin_ctx_t : public plugmod_t
{
	~plugin_ctx_t()
	{
		term_hexrays_plugin();
	}
	virtual bool idaapi run(size_t) override;
};

//--------------------------------------------------------------------------
static plugmod_t* idaapi init()
{
	if (!init_hexrays_plugin())
		return nullptr; // no decompiler
	const char* hxver = get_hexrays_version();
	msg("Hex-rays version %s has been detected, %s ready to use\n",
		hxver, PLUGIN.wanted_name);
	return new plugin_ctx_t;
}

 
std::map<std::string, std::string> function_names;

void find_and_print_calls(func_t* pfn) {
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr()) {
		ea_t ea = fii.current();
		insn_t insn;
		if (decode_insn(&insn, ea)) {
			if (insn.itype == 18 || insn.itype == 86 || insn.itype == 16) {
				qstring callee_name;
				if (get_name(&callee_name, insn.ops[0].addr) > 0) {
					msg("Call to function at address: %a, name: %s\n", insn.ops[0].addr, callee_name.c_str());
					function_names[callee_name.c_str()] = ""; // Save the function name  
				}
			}
		}
	}
}


bool rename_lvar2(ea_t func_ea, const char* oldname, const char* newname)
{
	lvar_saved_info_t info;
	if (!locate_lvar(&info.ll, func_ea, oldname))
		return false;
	info.name = newname;
	return modify_user_lvar_info(func_ea, MLI_NAME, info);
}


///////////////////////////////////////////////////////////////////////////////////////////
void rename_specific_function(const std::string& old_name, const std::string& new_name) {
	ea_t func_ea = get_name_ea(BADADDR, old_name.c_str());
	if (func_ea != BADADDR) {
		set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
		function_names[old_name] = new_name;
		msg("Function %s renamed to %s\n", old_name.c_str(), new_name.c_str());
	}
}

void rename_all_functions(func_t* pfn) {
	int counter = 1;
	for (auto& entry : function_names) {
		/*	if (entry.first == "print") {
				rename_specific_function("print", "print_print");
			}
			else {*/
		qstring new_name = qstring("function_") + std::to_string(counter).c_str();
		ea_t func_ea = get_name_ea(BADADDR, entry.first.c_str());
		if (func_ea != BADADDR) {
			set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
			entry.second = new_name.c_str();
			counter++;
		}
		//}
	}
	mark_cfunc_dirty(pfn->start_ea);
}
///////////////////////////////////////////////////////////////////////////////////////////


void rename_variables(func_t* pfn, const char* funcName) {
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr())
	{
		ea_t ea = fii.current();
		set_name(ea, "new_function_name", SN_NON_PUBLIC | SN_NON_WEAK | SN_NOLIST | SN_FORCE | SN_NODUMMY);
	}
	mark_cfunc_dirty(pfn->start_ea);
}

void analyze_function_elements(func_t* pfn) {
	// Получение декомпилированного кода  
	hexrays_failure_t hf;
	cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_WARNINGS);
	if (cfunc == nullptr) {
		warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
		return;
	}

	// Перебор локальных переменных  
	const lvars_t& lvars = *cfunc->get_lvars();
	for (auto& lvar : lvars) {
		if (lvar.is_arg_var()) {
			msg("Argument: %s\n", lvar.name.c_str());
		}
		else {
			msg("Local variable: %s\n", lvar.name.c_str());
		}
	}

	// Перебор всех элементов функции  
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr()) {
		ea_t ea = fii.current();
		if (is_code(get_flags(ea))) {
			// Проверка операндов инструкции на глобальные переменные  
			insn_t insn;
			decode_insn(&insn, ea);
			for (int i = 0; i < UA_MAXOP; i++) {
				if (insn.ops[i].type == o_mem) {
					qstring var_name;
					if (get_name(&var_name, insn.ops[i].addr) > 0) {
						msg("Global variable used at address: %a, name: %s\n", insn.ops[i].addr, var_name.c_str());
					}
					else {
						msg("Global variable used at address: %a\n", insn.ops[i].addr);
					}
				}
			}
		}
	}
}

void print_current_decompiled_code(func_t* pfn) {
	hexrays_failure_t hf;
	cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_WARNINGS);
	if (cfunc == nullptr)
	{
		warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
		return;
	}
	const strvec_t& sv = cfunc->get_pseudocode();
	for (int i = 0; i < sv.size(); i++)
	{
		qstring buf;
		tag_remove(&buf, sv[i].line);
		msg("%s\n", buf.c_str());
	}

}

// TODO :: 
// 1. Написать передачу декомпилированного кода в deepseekAI - print_current_decompiled_code()
// 2. Переименовать все переменные в функции и саму функцию -  find_function, fund_variables,deepseekAI получаем переименованную табличку из функций и переменных, rename_current_function()
// 3. Вывести на экран измененую функцию перед переименовывание, с подтверждением и возможность изменить функцию 
//--------------------------------------------------------------------------
bool idaapi plugin_ctx_t::run(size_t)
{
	ea_t screen_ea = get_screen_ea();
	func_t* pfn = get_func(get_screen_ea());
	if (pfn == nullptr)
	{
		warning("Please position the cursor within a function");
		return true;
	}
	msg("Current function start address: %a\n", pfn->start_ea);
	qstring funcName = "new_function_name";
	//renameFunction(pfn, funcName.c_str());
	//renameVariables(pfn, "sss");
	//analyzeFunctionElements(pfn);
	//getDecompiledCode(pfn);
	find_and_print_calls(pfn);

	const char* oldname = "v4";
	const char* newname = "var_444444";

	if (rename_lvar2(screen_ea, oldname, newname)) {
		msg("variable %s succes renamed %s\n", oldname, newname);
	}
	else {
		msg("error %s\n", oldname);
	}

	mark_cfunc_dirty(pfn->start_ea);
	return true;
}

//--------------------------------------------------------------------------
static char comment[] = "Sample1 plugin for Hex-Rays decompiler";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  PLUGIN_MULTI,         // The plugin can work with multiple idbs in parallel
  init,                 // initialize
  nullptr,
  nullptr,
  comment,              // long comment about the plugin
  nullptr,              // multiline help about the plugin
  "Decompile & Print",  // the preferred short name of the plugin
  nullptr,              // the preferred hotkey to run the plugin
};
