#include <tests/doctest.h>

#include <cmath>
#include <fstream>

#include <lfortran/fortran_evaluator.h>
#include <libasr/codegen/evaluator.h>
#include <libasr/exception.h>
#include <lfortran/ast.h>
#include <libasr/asr.h>
#include <lfortran/parser/parser.h>
#include <lfortran/semantics/ast_to_asr.h>
#include <libasr/codegen/asr_to_llvm.h>
#include <lfortran/pickle.h>
#include <libasr/pickle.h>
#include <libasr/utils.h>
#include <lfortran/utils.h>

using LCompilers::TRY;
using LCompilers::FortranEvaluator;
using LCompilers::CompilerOptions;

TEST_CASE("llvm 1") {
    //std::cout << "LLVM Version:" << std::endl;
    //LFortran::LLVMEvaluator::print_version_message();

    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
define i64 @f1()
{
    ret i64 4
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 4);
    e.add_module("");
//    CHECK(e.execfn<int64_t>("f1") == 4);

/*
    e.add_module(R"""(
define i64 @f1()
{
    ret i64 5
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 5);
    e.add_module("");
    CHECK(e.execfn<int64_t>("f1") == 5);
*/
}

TEST_CASE("llvm 1 fail") {
    LCompilers::LLVMEvaluator e;
    CHECK_THROWS_AS(e.add_module(R"""(
define i64 @f1()
{
    ; FAIL: "=x" is incorrect syntax
    %1 =x alloca i64
}
        )"""), LCompilers::LCompilersException);
    CHECK_THROWS_WITH(e.add_module(R"""(
define i64 @f1()
{
    ; FAIL: "=x" is incorrect syntax
    %1 =x alloca i64
}
        )"""), "parse_module(): Invalid LLVM IR");
}


TEST_CASE("llvm 2") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
@count = global i64 0

define i64 @f1()
{
    store i64 4, i64* @count
    %1 = load i64, i64* @count
    ret i64 %1
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 4);

    e.add_module(R"""(
@count = external global i64

define i64 @f2()
{
    %1 = load i64, i64* @count
    ret i64 %1
}
    )""");
    CHECK(e.execfn<int64_t>("f2") == 4);

    CHECK_THROWS_AS(e.add_module(R"""(
define i64 @f3()
{
    ; FAIL: @count is not defined
    %1 = load i64, i64* @count
    ret i64 %1
}
        )"""), LCompilers::LCompilersException);
}

TEST_CASE("llvm 3") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
@count = global i64 5
    )""");

    e.add_module(R"""(
@count = external global i64

define i64 @f1()
{
    %1 = load i64, i64* @count
    ret i64 %1
}

define void @inc()
{
    %1 = load i64, i64* @count
    %2 = add i64 %1, 1
    store i64 %2, i64* @count
    ret void
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 5);

//    e.execfn<void>("inc");
//    CHECK(e.execfn<int64_t>("f1") == 6);
/*    e.execfn<void>("inc");
    CHECK(e.execfn<int64_t>("f1") == 7);
    */
/*
    e.add_module(R"""(
@count = external global i64

define void @inc2()
{
    %1 = load i64, i64* @count
    %2 = add i64 %1, 2
    store i64 %2, i64* @count
    ret void
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 7);
    e.execfn<void>("inc2");
    CHECK(e.execfn<int64_t>("f1") == 9);
    e.execfn<void>("inc2");
    CHECK(e.execfn<int64_t>("f1") == 11);
    e.execfn<void>("inc2");
    CHECK(e.execfn<int64_t>("f1") == 13);

    // Test that we can have another independent LLVMEvaluator and use both at
    // the same time:
    LFortran::LLVMEvaluator e2;
    e2.add_module(R"""(
@count = global i64 5

define i64 @f1()
{
    %1 = load i64, i64* @count
    ret i64 %1
}

define void @inc()
{
    %1 = load i64, i64* @count
    %2 = add i64 %1, 1
    store i64 %2, i64* @count
    ret void
}
    )""");

    CHECK(e2.execfn<int64_t>("f1") == 5);
    e2.execfn<void>("inc");
    CHECK(e2.execfn<int64_t>("f1") == 6);
    e2.execfn<void>("inc");
    CHECK(e2.execfn<int64_t>("f1") == 7);

    CHECK(e.execfn<int64_t>("f1") == 12);
    e2.execfn<void>("inc");
    CHECK(e2.execfn<int64_t>("f1") == 8);
    CHECK(e.execfn<int64_t>("f1") == 12);
    e.execfn<void>("inc2");
    CHECK(e2.execfn<int64_t>("f1") == 8);
    CHECK(e.execfn<int64_t>("f1") == 14);
*/
}

TEST_CASE("llvm 4") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
@count = global i64 5

define i64 @f1()
{
    %1 = load i64, i64* @count
    ret i64 %1
}

define void @inc()
{
    %1 = load i64, i64* @count
    %2 = add i64 %1, 1
    store i64 %2, i64* @count
    ret void
}
)""");
    CHECK(e.execfn<int64_t>("f1") == 5);
/*    e.execfn<void>("inc");
    CHECK(e.execfn<int64_t>("f1") == 6);
    e.execfn<void>("inc");
    CHECK(e.execfn<int64_t>("f1") == 7);
    */
/*
    e.add_module(R"""(
declare void @inc()

define void @inc2()
{
    call void @inc()
    call void @inc()
    ret void
}
)""");
    CHECK(e.execfn<int64_t>("f1") == 7);
    e.execfn<void>("inc2");
    CHECK(e.execfn<int64_t>("f1") == 9);
    e.execfn<void>("inc");
    CHECK(e.execfn<int64_t>("f1") == 10);
    e.execfn<void>("inc2");
    CHECK(e.execfn<int64_t>("f1") == 12);

    CHECK_THROWS_AS(e.add_module(R"""(
define void @inc2()
{
    ; FAIL: @inc is not defined
    call void @inc()
    call void @inc()
    ret void
}
        )"""), LFortran::LCompilersException);
        */
}

TEST_CASE("llvm array 1") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
; Sum the three elements in %a
define i64 @sum3(i64* %a)
{
    %a1addr = getelementptr i64, i64* %a, i64 0
    %a1 = load i64, i64* %a1addr

    %a2addr = getelementptr i64, i64* %a, i64 1
    %a2 = load i64, i64* %a2addr

    %a3addr = getelementptr i64, i64* %a, i64 2
    %a3 = load i64, i64* %a3addr

    %tmp = add i64 %a2, %a1
    %r = add i64 %a3, %tmp

    ret i64 %r
}


define i64 @f()
{
    %a = alloca [3 x i64]

    %a1 = getelementptr [3 x i64], [3 x i64]* %a, i64 0, i64 0
    store i64 1, i64* %a1

    %a2 = getelementptr [3 x i64], [3 x i64]* %a, i64 0, i64 1
    store i64 2, i64* %a2

    %a3 = getelementptr [3 x i64], [3 x i64]* %a, i64 0, i64 2
    store i64 3, i64* %a3

    %r = call i64 @sum3(i64* %a1)
    ret i64 %r
}
    )""");
    CHECK(e.execfn<int64_t>("f") == 6);
}

TEST_CASE("llvm array 2") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
%array = type {i64, [3 x i64]}

; Sum the three elements in %a
define i64 @sum3(%array* %a)
{
    %a1addr = getelementptr %array, %array* %a, i64 0, i32 1, i64 0
    %a1 = load i64, i64* %a1addr

    %a2addr = getelementptr %array, %array* %a, i64 0, i32 1, i64 1
    %a2 = load i64, i64* %a2addr

    %a3addr = getelementptr %array, %array* %a, i64 0, i32 1, i64 2
    %a3 = load i64, i64* %a3addr

    %tmp = add i64 %a2, %a1
    %r = add i64 %a3, %tmp

    ret i64 %r
}


define i64 @f()
{
    %a = alloca %array

    %idx0 = getelementptr %array, %array* %a, i64 0, i32 0
    store i64 3, i64* %idx0

    %idx1 = getelementptr %array, %array* %a, i64 0, i32 1, i64 0
    store i64 1, i64* %idx1
    %idx2 = getelementptr %array, %array* %a, i64 0, i32 1, i64 1
    store i64 2, i64* %idx2
    %idx3 = getelementptr %array, %array* %a, i64 0, i32 1, i64 2
    store i64 3, i64* %idx3

    %r = call i64 @sum3(%array* %a)
    ret i64 %r
}
    )""");
    //CHECK(e.execfn<int64_t>("f") == 6);
}

int f(int a, int b) {
    return a+b;
}

TEST_CASE("llvm callback 0") {
    LCompilers::LLVMEvaluator e;
    std::string addr = std::to_string((int64_t)f);
    e.add_module(R"""(
define i64 @addrcaller(i64 %a, i64 %b)
{
    %f = inttoptr i64 )""" + addr + R"""( to i64 (i64, i64)*
    %r = call i64 %f(i64 %a, i64 %b)
    ret i64 %r
}

define i64 @f1()
{
    %r = call i64 @addrcaller(i64 2, i64 3)
    ret i64 %r
}
    )""");
    CHECK(e.execfn<int64_t>("f1") == 5);
}


TEST_CASE("ASR -> LLVM 1") {
    std::string source = R"(function f()
integer :: f
f = 5
end function)";

    // Src -> AST
    Allocator al(4*1024);
    LCompilers::diag::Diagnostics diagnostics;
    CompilerOptions compiler_options;
    compiler_options.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    LCompilers::LFortran::AST::TranslationUnit_t* tu = TRY(LCompilers::LFortran::parse(al, source,
        diagnostics, compiler_options));
    LCompilers::LFortran::AST::ast_t* ast = tu->m_items[0];
    CHECK(LCompilers::LFortran::pickle(*ast) == "(Function f [] [] () () () [] [] [] [(Declaration (AttrType TypeInteger [] () () None) [] [(f [] [] () () None ())] ())] [(Assignment 0 f 5 ())] [] [])");

    // AST -> ASR
    LCompilers::SymbolTable::reset_global_counter();
    LCompilers::LocationManager lm;
    LCompilers::ASR::TranslationUnit_t* asr = TRY(LCompilers::LFortran::ast_to_asr(al, *tu,
        diagnostics, nullptr, false, compiler_options, lm));
    CHECK(LCompilers::pickle(*asr) == "(TranslationUnit (SymbolTable 1 {f: (Function (SymbolTable 2 {f: (Variable 2 f [] ReturnVar () () Default (Integer 4) () Source Public Required .false. .false. .false. () .false. .false.)}) f (FunctionType [] (Integer 4) Source Implementation () .false. .false. .false. .false. .false. [] .false.) [] [] [(Assignment (Var 2 f) (IntegerConstant 5 (Integer 4) Decimal) () .false.)] (Var 2 f) Public .false. .false. ())}) [])");

    // ASR -> LLVM
    LCompilers::LLVMEvaluator e;
    LCompilers::PassManager lpm;
    lpm.use_default_passes();
    CompilerOptions co;
    co.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    co.platform = LCompilers::get_platform();
    LCompilers::Result<std::unique_ptr<LCompilers::LLVMModule>>
        res = LCompilers::asr_to_llvm(*asr, diagnostics, e.get_context(), al,
            lpm, co, "f", "", "");
    REQUIRE(res.ok);
    std::unique_ptr<LCompilers::LLVMModule> m = std::move(res.result);
    //std::cout << "Module:" << std::endl;
    //std::cout << m->str() << std::endl;

    // LLVM -> Machine code -> Execution
    e.add_module(std::move(m));
    CHECK(e.execfn<int32_t>("f") == 5);
}

TEST_CASE("ASR -> LLVM 2") {
    std::string source = R"(function f()
integer :: f
f = 4
end function)";

    // Src -> AST
    Allocator al(4*1024);
    LCompilers::diag::Diagnostics diagnostics;
    CompilerOptions compiler_options;
    compiler_options.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    LCompilers::LFortran::AST::TranslationUnit_t* tu = TRY(LCompilers::LFortran::parse(al, source,
        diagnostics, compiler_options));
    LCompilers::LFortran::AST::ast_t* ast = tu->m_items[0];
    CHECK(LCompilers::LFortran::pickle(*ast) == "(Function f [] [] () () () [] [] [] [(Declaration (AttrType TypeInteger [] () () None) [] [(f [] [] () () None ())] ())] [(Assignment 0 f 4 ())] [] [])");

    // AST -> ASR
    LCompilers::LocationManager lm;
    LCompilers::ASR::TranslationUnit_t* asr = TRY(LCompilers::LFortran::ast_to_asr(al, *tu,
        diagnostics, nullptr, false, compiler_options, lm));
    CHECK(LCompilers::pickle(*asr) == "(TranslationUnit (SymbolTable 3 {f: (Function (SymbolTable 4 {f: (Variable 4 f [] ReturnVar () () Default (Integer 4) () Source Public Required .false. .false. .false. () .false. .false.)}) f (FunctionType [] (Integer 4) Source Implementation () .false. .false. .false. .false. .false. [] .false.) [] [] [(Assignment (Var 4 f) (IntegerConstant 4 (Integer 4) Decimal) () .false.)] (Var 4 f) Public .false. .false. ())}) [])");
    // ASR -> LLVM
    LCompilers::LLVMEvaluator e;
    LCompilers::PassManager lpm;
    lpm.use_default_passes();
    LCompilers::Result<std::unique_ptr<LCompilers::LLVMModule>>
        res = LCompilers::asr_to_llvm(*asr, diagnostics, e.get_context(), al,
            lpm, compiler_options, "f", "", "");
    REQUIRE(res.ok);
    std::unique_ptr<LCompilers::LLVMModule> m = std::move(res.result);
    //std::cout << "Module:" << std::endl;
    //std::cout << m->str() << std::endl;

    // LLVM -> Machine code -> Execution
    e.add_module(std::move(m));
    CHECK(e.execfn<int32_t>("f") == 4);
}

TEST_CASE("FortranEvaluator 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);
}

TEST_CASE("FortranEvaluator 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(real :: r
r = 3
r
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(r.result.f32 == 3);
}

TEST_CASE("FortranEvaluator 3") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    e.evaluate2("integer :: i, j");
    e.evaluate2(R"(j = 0
do i = 1, 5
    j = j + i
end do
)");
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("j");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 15);
}

TEST_CASE("FortranEvaluator 4") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    e.evaluate2(R"(
integer function fn(i, j)
integer, intent(in) :: i, j
fn = i + j
end function
)");
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("fn(2, 3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);

/*
    e.evaluate2(R"(
integer function fn(i, j)
integer, intent(in) :: i, j
fn = i - j
end function
)");
    r = e.evaluate2("fn(2, 3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == -1);
*/
}

TEST_CASE("FortranEvaluator 5") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    e.evaluate2(R"(
integer subroutine fn(i, j, r)
integer, intent(in) :: i, j
integer, intent(out) :: r
r = i + j
end subroutine
)");
    e.evaluate2("integer :: r");
    e.evaluate2("call fn(2, 3, r)");
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("r");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);

/*
    e.evaluate2(R"(
integer subroutine fn(i, j, r)
integer, intent(in) :: i, j
integer, intent(out) :: r
r = i - j
end subroutine
)");
    e.evaluate2("call fn(2, 3, r)");
    r = e.evaluate2("r");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == -1);
    */
}

TEST_CASE("FortranEvaluator 6") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);

    LCompilers::LocationManager lm;
    LCompilers::PassManager lpm;
    lpm.use_default_passes();
    {
        LCompilers::LocationManager::FileLocations fl;
        fl.in_filename = "input.f90";
        lm.files.push_back(fl);
    }
    LCompilers::diag::Diagnostics diagnostics;

    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate("$", false, lm, lpm, diagnostics);
    CHECK(!r.ok);
    REQUIRE(diagnostics.diagnostics.size() >= 1);
    CHECK(diagnostics.diagnostics[0].stage == LCompilers::diag::Stage::Tokenizer);
    diagnostics.diagnostics.clear();

    r = e.evaluate("1x", false, lm, lpm, diagnostics);
    CHECK(!r.ok);
    REQUIRE(diagnostics.diagnostics.size() >= 1);
    CHECK(diagnostics.diagnostics[0].stage == LCompilers::diag::Stage::Parser);
    diagnostics.diagnostics.clear();

    r = e.evaluate("x = 'x'", false, lm, lpm, diagnostics);
    CHECK(!r.ok);
    REQUIRE(diagnostics.diagnostics.size() >= 1);
    CHECK(diagnostics.diagnostics[0].stage == LCompilers::diag::Stage::Semantic);
    diagnostics.diagnostics.clear();
}

TEST_CASE("FortranEvaluator 6 importing modules") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);

    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(module funcmod
    implicit none

    contains

    function add(x,y)
        real :: x, y
        real :: add
        add = x+y
    end function add
    function subtract(x,y)
        real :: x, y
        real :: subtract
        subtract = x-y
    end function subtract
end module funcmod)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);

    r = e.evaluate2("use funcmod, only : add, subtract");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);

    r = e.evaluate2("add(2.0, 5.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(abs(r.result.f32 - 7.0) <= 1e-8 );

    r = e.evaluate2("subtract(2.0, 5.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(abs(r.result.f32 - (-3.0)) <= 1e-8 );
}

// Tests passing the complex struct by reference
TEST_CASE("llvm complex type") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
%complex = type { float, float }

define float @sum2(%complex* %a)
{
    %a1addr = getelementptr %complex, %complex* %a, i32 0, i32 0
    %a1 = load float, float* %a1addr

    %a2addr = getelementptr %complex, %complex* %a, i32 0, i32 1
    %a2 = load float, float* %a2addr

    %r = fadd float %a2, %a1

    ret float %r
}

define float @f()
{
    %a = alloca %complex

    %a1 = getelementptr %complex, %complex* %a, i32 0, i32 0
    store float 3.5, float* %a1

    %a2 = getelementptr %complex, %complex* %a, i32 0, i32 1
    store float 4.5, float* %a2

    %r = call float @sum2(%complex* %a)
    ret float %r
}
    )""");
    CHECK(std::abs(e.execfn<float>("f") - 8) < 1e-6);
}

// Tests passing the complex struct by value
TEST_CASE("llvm complex type value") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
%complex = type { float, float }

define float @sum2(%complex %a_value)
{
    %a = alloca %complex
    store %complex %a_value, %complex* %a

    %a1addr = getelementptr %complex, %complex* %a, i32 0, i32 0
    %a1 = load float, float* %a1addr

    %a2addr = getelementptr %complex, %complex* %a, i32 0, i32 1
    %a2 = load float, float* %a2addr

    %r = fadd float %a2, %a1
    ret float %r
}

define float @f()
{
    %a = alloca %complex

    %a1 = getelementptr %complex, %complex* %a, i32 0, i32 0
    store float 3.5, float* %a1

    %a2 = getelementptr %complex, %complex* %a, i32 0, i32 1
    store float 4.5, float* %a2

    %ap = load %complex, %complex* %a
    %r = call float @sum2(%complex %ap)
    ret float %r
}
    )""");
//    CHECK(std::abs(e.execfn<float>("f") - 8) < 1e-6);
}

// Tests passing boolean by reference
TEST_CASE("llvm boolean type") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(

define i1 @and_func(i1* %p, i1* %q)
{
    %pval = load i1, i1* %p
    %qval = load i1, i1* %q

    %r = and i1 %pval, %qval
    ret i1 %r
}

define i1 @b()
{
    %p = alloca i1
    %q = alloca i1

    store i1 1, i1* %p
    store i1 0, i1* %q

    %r = call i1 @and_func(i1* %p, i1* %q)

    ret i1 %r
}
    )""");
    CHECK(e.execfn<bool>("b") == false);
}

// Tests passing boolean by value
TEST_CASE("llvm boolean type") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(

define i1 @and_func(i1 %p, i1 %q)
{
    %r = and i1 %p, %q
    ret i1 %r
}

define i1 @b()
{
    %p = alloca i1
    %q = alloca i1

    store i1 1, i1* %p
    store i1 0, i1* %q

    %pval = load i1, i1* %p
    %qval = load i1, i1* %q

    %r = call i1 @and_func(i1 %pval, i1 %qval)

    ret i1 %r
}
    )""");
    CHECK(e.execfn<bool>("b") == false);
}

// Tests pointers
TEST_CASE("llvm pointers 1") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
@r = global i64 0

define i64 @f()
{
    store i64 8, i64* @r

    ; Dereferences the pointer %r
    ;%rval = load i64, i64* @r

    %raddr = ptrtoint i64* @r to i64

    ret i64 %raddr
}
    )""");
    int64_t r = e.execfn<int64_t>("f");
    CHECK(r != 8);
    int64_t *p = (int64_t*)r;
    CHECK(*p == 8);
}

TEST_CASE("llvm pointers 2") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
@r = global float 0.0

define i64 @f()
{
    store float 8.0, float* @r

    %raddr = ptrtoint float* @r to i64

    ret i64 %raddr
}
    )""");
    int64_t r = e.execfn<int64_t>("f");
    float *p = (float *)r;
    CHECK(std::abs(*p - 8) < 1e-6);
}

TEST_CASE("llvm pointers 3") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
; Takes a variable and returns a pointer to it
define i64 @pointer_reference(float* %var)
{
    %ret = ptrtoint float* %var to i64
    ret i64 %ret
}

; Takes a pointer and returns the value of what it points to
define float @pointer_dereference(i64 %p)
{
    %p2 = inttoptr i64 %p to float*
    %ret = load float, float* %p2
    ret float %ret
}

define float @f()
{
    %var = alloca float
    store float 8.0, float* %var

    %pointer_val = call i64 @pointer_reference(float* %var)

    ; Save the pointer to a variable
    %pointer_var = alloca i64
    store i64 %pointer_val, i64* %pointer_var
    ; Extract value out of the pointer %pointer_var
    %pointer_val2 = load i64, i64* %pointer_var

    %ret = call float @pointer_dereference(i64 %pointer_val2)

    ret float %ret
}
    )""");
    float r = e.execfn<float>("f");
    CHECK(std::abs(r - 8) < 1e-6);
}

TEST_CASE("llvm pointers 4") {
    LCompilers::LLVMEvaluator e;
    e.add_module(R"""(
; Takes a variable and returns a pointer to it
define float* @pointer_reference(float* %var)
{
    ret float* %var
}

define float @pointer_dereference(float* %var)
{
    %ret = load float, float* %var
    ret float %ret
}

define float @f()
{
    %var = alloca float
    store float 8.0, float* %var

    %pointer_val = call float* @pointer_reference(float* %var)

    ; Save the pointer to a variable
    %pointer_var = alloca float*
    store float* %pointer_val, float** %pointer_var
    ; Extract value out of the pointer %pointer_var
    %pointer_val2 = load float*, float** %pointer_var

    %ret = call float @pointer_dereference(float* %pointer_val2)

    ret float %ret
}
    )""");
    float r = e.execfn<float>("f");
    CHECK(std::abs(r - 8) < 1e-6);
}

TEST_CASE("FortranEvaluator 7") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer :: i = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);
}

TEST_CASE("FortranEvaluator 8") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("real :: a = 3.5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("a");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(r.result.f32 == 3.5);
}

TEST_CASE("FortranEvaluator 8 double") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("real(8) :: a = 3.5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("a");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real8);
    CHECK(r.result.f64 == 3.5);
}

TEST_CASE("FortranEvaluator 9 single complex") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    if (cu.platform == LCompilers::Platform::Linux) {
        FortranEvaluator e(cu);
        LCompilers::Result<FortranEvaluator::EvalResult>
        r = e.evaluate2("(2.5_4, 3.5_4)");
        CHECK(r.ok);
        CHECK(r.result.type == FortranEvaluator::EvalResult::complex4);
        CHECK(r.result.c32.re == 2.5);
        CHECK(r.result.c32.im == 3.5);
    }
}

TEST_CASE("FortranEvaluator 9 double complex") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    if (cu.platform != LCompilers::Platform::Windows) {
        FortranEvaluator e(cu);
        LCompilers::Result<FortranEvaluator::EvalResult>
        r = e.evaluate2("(2.5_8, 3.5_8)");
        CHECK(r.ok);
        CHECK(r.result.type == FortranEvaluator::EvalResult::complex8);
        CHECK(r.result.c64.re == 2.5);
        CHECK(r.result.c64.im == 3.5);
    }
}

TEST_CASE("FortranEvaluator logical 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("logical :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = .true.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(r.result.b);
}

TEST_CASE("FortranEvaluator logical 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("logical :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = .false.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(!r.result.b);
}

TEST_CASE("FortranEvaluator logical 3") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(".false. .or. .true.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(r.result.b);
    r = e.evaluate2(".false. .and. .true.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(!r.result.b);
    r = e.evaluate2("logical :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = .false.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i .or. .true.");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(r.result.b);
}

TEST_CASE("FortranEvaluator logical 4") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(
function is_even(n) result(result)
integer, intent(in) :: n
logical :: result
result = mod(n, 2) == 0
end function is_even
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("is_even(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(!r.result.b);
    r = e.evaluate2("is_even(4)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::boolean);
    CHECK(r.result.b);
}

TEST_CASE("FortranEvaluator integer kind 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer(4) :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);
}

TEST_CASE("FortranEvaluator integer kind 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer(8) :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer8);
    CHECK(r.result.i64 == 5);
}

TEST_CASE("FortranEvaluator Array 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer :: i(10)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("print *, i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
}

TEST_CASE("FortranEvaluator Array 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer :: x(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("x = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("print *, x");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
}

TEST_CASE("FortranEvaluator re-declaration 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("integer :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = 5");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 5);

/*
    r = e.evaluate2("integer :: i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("i = 6");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("i");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 6);
*/
}

TEST_CASE("FortranEvaluator re-declaration 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(
integer function fn(i)
integer, intent(in) :: i
fn = i+1
end function
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("fn(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 4);

/*
    r = e.evaluate2(R"(
integer function fn(i)
integer, intent(in) :: i
fn = i-1
end function
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("fn(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 2);
*/
}

TEST_CASE("FortranEvaluator asr verify 1") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(
pure function double(x) result(r)
    integer, intent(in) :: x
    integer :: r
    r = 2 * x
end function double
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2(R"(
subroutine s(n, x)
    integer, intent(in) :: n
    integer, intent(inout) :: x
    x = double(n)
end subroutine s
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("integer :: x");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("x = double(1)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("x");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 2);
    r = e.evaluate2("call s(x, x)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("x");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 4);
}

TEST_CASE("FortranEvaluator asr verify 2") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(
pure function id(x) result(r)
    integer, intent(in) :: x
    integer :: r
    r = x
end function id
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2(R"(
subroutine sa(l, a)
    integer, intent(in) :: l
    integer, intent(inout) :: a(id(l))

    integer :: i

    do i = 1, size(a)
        a(i) = a(i) + 1
    end do
end subroutine sa
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("integer :: arr(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("arr = 0");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("call sa(3, arr)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::statement);
    r = e.evaluate2("arr(1)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 1);
    r = e.evaluate2("arr(2)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 1);
    r = e.evaluate2("arr(3)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::integer4);
    CHECK(r.result.i32 == 1);
}

TEST_CASE("FortranEvaluator asr verify 3") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2(R"(
function add(x, y) result(r)
    real, intent(in) :: x
    real, intent(in) :: y
    real :: r
    r = x + y
end function add
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2(R"(
function sub(x, y) result(r)
    real, intent(in) :: x
    real, intent(in) :: y
    real :: r
    r = add(x, -y)
end function sub
)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::none);
    r = e.evaluate2("add(2.0, 3.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(r.result.f32 == 5.0);
    r = e.evaluate2("sub(2.0, 3.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(r.result.f32 == -1.0);
}

TEST_CASE("llvm ir 1") {
    LCompilers::LLVMEvaluator e;
    std::string file_name = std::string(LFORTRAN_PROJECT_SOURCE_DIR) + "/src/lfortran/tests/ir.ll";
    std::ifstream infile(file_name);
    if (!infile.good()) {
        std::string error_msg = "File '" + file_name + "' doesn't exist or isn't readable";
        FAIL(error_msg);
    }
    // `file_name` evaluates to something like:
    // "/Users/gxyd/OpenSource/lfortran/lfortran-${version}/src/lfortran/tests/ir.ll",
    // where `version` isn't a local variable
    CHECK_THROWS_AS(e.parse_module2("", file_name), LCompilers::LCompilersException);
    CHECK_THROWS_WITH(e.parse_module2("", file_name), "parse_module(): Invalid LLVM IR");
}

// This test does not work on Windows yet
// https://github.com/lfortran/lfortran/issues/913
#if !defined(_WIN32)
TEST_CASE("FortranEvaluator 10 trig functions") {
    CompilerOptions cu;
    cu.interactive = true;
    cu.po.runtime_library_dir = LCompilers::LFortran::get_runtime_library_dir();
    FortranEvaluator e(cu);
    LCompilers::Result<FortranEvaluator::EvalResult>
    r = e.evaluate2("sin(1.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(std::abs(r.result.f32 - 0.8414709848078965) < 1e-7);
    /*
    r = e.evaluate2("sin(1.d0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real8);
    CHECK(std::abs(r.result.f64 - 0.8414709848078965) < 1e-14);
    r = e.evaluate2("cos(1.0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real4);
    CHECK(std::abs(r.result.f32 - 0.5403023058681398) < 1e-7);
    r = e.evaluate2("cos(1.d0)");
    CHECK(r.ok);
    CHECK(r.result.type == FortranEvaluator::EvalResult::real8);
    CHECK(std::abs(r.result.f64 - 0.5403023058681398) < 1e-14);
    */
}
#endif
