#include <libasr/asr.h>
#include <libasr/location.h>
#include <libasr/asr_utils.h>
#include <libasr/pass/intrinsic_function_registry.h>
#include <libasr/pass/intrinsic_subroutine_registry.h>
#include <libasr/pass/intrinsic_array_function_registry.h>
#include <libasr/asr_pickle_visitor.h>
#include <libasr/asr_json_visitor.h>
#include <libasr/asr_tree_visitor.h>

namespace LCompilers {

/********************** ASR Pickle *******************/
class ASRPickleVisitor :
    public ASR::PickleBaseVisitor<ASRPickleVisitor>
{
public:
    bool show_intrinsic_modules;

    std::string get_str() {
        return s;
    }
    void visit_symbol(const ASR::symbol_t &x) {
        s.append(ASRUtils::symbol_parent_symtab(&x)->get_counter());
        s.append(" ");
        if (use_colors) {
            s.append(color(fg::yellow));
        }
        s.append(ASRUtils::symbol_name(&x));
        if (use_colors) {
            s.append(color(fg::reset));
        }
    }
    void visit_IntegerConstant(const ASR::IntegerConstant_t &x) {
        s.append("(");
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::magenta));
        }
        s.append("IntegerConstant");
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        s.append(" ");
        if (use_colors) {
            s.append(color(fg::cyan));
        }
        s.append(std::to_string(x.m_n));
        if (use_colors) {
            s.append(color(fg::reset));
        }
        s.append(" ");
        this->visit_ttype(*x.m_type);
        s.append(" ");
        this->visit_integerbozType(x.m_intboz_type);
        s.append(")");
    }
    void visit_Module(const ASR::Module_t &x) {
        if (!show_intrinsic_modules &&
            (x.m_intrinsic || startswith(x.m_name, "numpy"))) {
            s.append("(");
            if (use_colors) {
                s.append(color(style::bold));
                s.append(color(fg::magenta));
            }
            s.append(x.m_intrinsic ? "IntrinsicModule" : "Module");
            if (use_colors) {
                s.append(color(fg::reset));
                s.append(color(style::reset));
            }
            s.append(" ");
            s.append(x.m_name);
            s.append(")");
        } else {
            ASR::PickleBaseVisitor<ASRPickleVisitor>::visit_Module(x);
        };
    }
    void visit_ArrayConstant(const ASR::ArrayConstant_t &x) {
        s.append("(");
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::magenta));
        }
        s.append("ArrayConstant");
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        if(indent) {
            inc_indent();
            s.append("\n" + indented);
        } else {
            s.append(" ");
        }
        s.append(std::to_string(x.m_n_data));
        if(indent) s.append("\n" + indented);
        else s.append(" ");
        s.append("[");
        int kind;
        if(ASRUtils::is_character(*x.m_type)){
            ASR::String_t* str = ASR::down_cast<ASR::String_t>(
                ASRUtils::type_get_past_array(x.m_type));
            if(!ASRUtils::extract_value(str->m_len, kind)){LCOMPILERS_ASSERT(false)}
        } else {
            kind = ASRUtils::extract_kind_from_ttype_t(x.m_type);
        }
        int size = x.m_n_data / kind;
        int curr = 0;
        for (int i = 0; i < 3; i++) {
            if (curr < size) {
                if (i > 0) s.append(", ");
                s.append(ASRUtils::fetch_ArrayConstant_value(x, curr));
                curr++;
            }
        }
        if (size > 6) {
            s.append(", ....");
            curr = size - 3;
        }
        for (int i = 0; i < 3; i++) {
            if (curr < size) {
                s.append(", ");
                s.append(ASRUtils::fetch_ArrayConstant_value(x, curr));
                curr++;
            }
        }
        s.append("]");
        if(indent) s.append("\n" + indented);
        else s.append(" ");
        this->visit_ttype(*x.m_type);
        if(indent) s.append("\n" + indented);
        else s.append(" ");
        visit_arraystorageType(x.m_storage_format);
        if(indent) {
            dec_indent();
            s.append("\n" + indented);
        }
        s.append(")");
    }

    std::string convert_intrinsic_id(int x) {
        std::string s;
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::green));
        }
        s.append(ASRUtils::get_intrinsic_name(x));
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        return s;
    }

    std::string convert_sub_intrinsic_id(int x) {
        std::string s;
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::green));
        }
        s.append(ASRUtils::get_intrinsic_subroutine_name(x));
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        return s;
    }

    std::string convert_impure_intrinsic_id(int x) {
        std::string s;
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::green));
        }
        s.append(ASRUtils::get_impure_intrinsic_name(x));
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        return s;
    }

    std::string convert_array_intrinsic_id(int x) {
        std::string s;
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::green));
        }
        s.append(ASRUtils::get_array_intrinsic_name(x));
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        return s;
    }
};

std::string pickle(ASR::asr_t &asr, bool colors, bool indent,
        bool show_intrinsic_modules) {
    ASRPickleVisitor v;
    v.use_colors = colors;
    v.indent = indent;
    v.show_intrinsic_modules = show_intrinsic_modules;
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle(ASR::TranslationUnit_t &asr, bool colors, bool indent, bool show_intrinsic_modules) {
    return pickle((ASR::asr_t &)asr, colors, indent, show_intrinsic_modules);
}

/********************** ASR Pickle Tree *******************/
class ASRTreeVisitor :
    public ASR::TreeBaseVisitor<ASRTreeVisitor>
{
public:
    bool show_intrinsic_modules;

    std::string get_str() {
        return s;
    }

};

std::string pickle_tree(ASR::asr_t &asr, bool colors, bool show_intrinsic_modules) {
    ASRTreeVisitor v;
    v.use_colors = colors;
    v.show_intrinsic_modules = show_intrinsic_modules;
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle_tree(ASR::TranslationUnit_t &asr, bool colors, bool show_intrinsic_modules) {
    return pickle_tree((ASR::asr_t &)asr, colors, show_intrinsic_modules);
}

/********************** ASR Pickle Json *******************/
class ASRJsonVisitor :
    public ASR::JsonBaseVisitor<ASRJsonVisitor>
{
public:
    bool show_intrinsic_modules;

    using ASR::JsonBaseVisitor<ASRJsonVisitor>::JsonBaseVisitor;

    std::string get_str() {
        return s;
    }

    void visit_symbol(const ASR::symbol_t &x) {
        s.append("\"");
        s.append(ASRUtils::symbol_name(&x));
        s.append(" (SymbolTable");
        s.append(ASRUtils::symbol_parent_symtab(&x)->get_counter());
        s.append(")\"");
    }

    void visit_Module(const ASR::Module_t &x) {
        if (x.m_intrinsic && !show_intrinsic_modules) { // do not show intrinsic modules by default
            s.append("{");
            inc_indent(); s.append("\n" + indtd);
            s.append("\"node\": \"Module\"");
            s.append(",\n" + indtd);
            s.append("\"fields\": {");
            inc_indent(); s.append("\n" + indtd);
            s.append("\"name\": ");
            s.append("\"" + std::string(x.m_name) + "\"");
            s.append(",\n" + indtd);
            s.append("\"parent_module\": ");
            if (x.m_parent_module) {
                s.append("\"" + std::string(x.m_parent_module) + "\"");
            } else {
                s.append("[]");
            }
            s.append(",\n" + indtd);
            s.append("\"dependencies\": ");
            s.append("[");
            if (x.n_dependencies > 0) {
                inc_indent(); s.append("\n" + indtd);
                for (size_t i=0; i<x.n_dependencies; i++) {
                    s.append("\"" + std::string(x.m_dependencies[i]) + "\"");
                    if (i < x.n_dependencies-1) {
                        s.append(",\n" + indtd);
                    };
                }
                dec_indent(); s.append("\n" + indtd);
            }
            s.append("]");
            s.append(",\n" + indtd);
            s.append("\"loaded_from_mod\": ");
            if (x.m_loaded_from_mod) {
                s.append("true");
            } else {
                s.append("false");
            }
            s.append(",\n" + indtd);
            s.append("\"intrinsic\": ");
            if (x.m_intrinsic) {
                s.append("true");
            } else {
                s.append("false");
            }
            s.append(",\n" + indtd);
            s.append("\"has_submodules\": ");
            if (x.m_has_submodules) {
                s.append("true");
            } else {
                s.append("false");
            }
            dec_indent(); s.append("\n" + indtd);
            s.append("}");
            append_location(s, x.base.base.loc.first, x.base.base.loc.last);
            dec_indent(); s.append("\n" + indtd);
            s.append("}");
        } else {
            ASR::JsonBaseVisitor<ASRJsonVisitor>::visit_Module(x);
        }
    }
};

std::string pickle_json(ASR::asr_t &asr, LocationManager &lm, bool no_loc, bool show_intrinsic_modules) {
    ASRJsonVisitor v(lm);
    v.show_intrinsic_modules = show_intrinsic_modules;
    v.no_loc = no_loc;
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle_json(ASR::TranslationUnit_t &asr, LocationManager &lm, bool no_loc, bool show_intrinsic_modules) {
    return pickle_json((ASR::asr_t &)asr, lm, no_loc, show_intrinsic_modules);
}

} // namespace LCompilers
