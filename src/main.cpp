#include <sstream>
#include <string>
//new
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <system_error>
#include <iostream>
#include <algorithm>
#include <cctype>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/raw_ostream.h"
//new
//llvm::cl::extrahelp
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Frontend/Rewriters.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
std::string UnaryOperatorAsString(Expr *E);
std::string BinaryOperatorAsString(Stmt *st);
bool hasBreak(std::string &str)
{
	auto n = str.find("break;");
	return !(n == std::string::npos);
}
void eraseBreak(std::string &str)
{
	auto n = str.find("break;");
	if (n != std::string::npos)
		str.erase(n, 6);
}
bool Switchtoif(Stmt *s);
// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
	// RecursiveASTVisitor�ฺ��ʵ�ʶ�Դ��ĸ�д
public:
	MyASTVisitor(Rewriter &R, CompilerInstance &Ci) : TheRewriter(R), TheCompiler(Ci) {}
	//MyASTVisitor(Rewriter &R) : TheRewriter(R) { }

	bool VisitStmt(Stmt *s)
	{

		// Only care about If statements.
		if (isa<IfStmt>(s))
		{
			IfStmt *IfStatement = cast<IfStmt>(s);

			Stmt *Then = IfStatement->getThen();
			Stmt *Else = IfStatement->getElse();
			if (isa<CompoundStmt>(Then)) // part Then
			{
				if (Else)
				{
					if (!isa<CompoundStmt>(Else)) // else a++;
					{
						TheRewriter.InsertText(Else->getBeginLoc(), "{\n", true, true);
						SourceLocation END = Else->getEndLoc();
						int offset = Lexer::MeasureTokenLength(END,
															   TheRewriter.getSourceMgr(),
															   TheRewriter.getLangOpts()) +
									 1;
						SourceLocation END1 = END.getLocWithOffset(offset);
						TheRewriter.InsertText(END1, "\n}", true, true);
					}
				}
				else
				{
					TheRewriter.InsertText(Then->getEndLoc().getLocWithOffset(1), "else {}\n", true, true);
				}
			}
			else
			{
				if (Else)
				{
					if (!isa<CompoundStmt>(Else)) // if a++ else a++ |-> if {a++} else {a++}
					{
						TheRewriter.InsertText(Then->getBeginLoc(), "{\n", true, true); // if {a++
						SourceLocation END = Then->getEndLoc();
						int offset = Lexer::MeasureTokenLength(END,
															   TheRewriter.getSourceMgr(),
															   TheRewriter.getLangOpts()) +
									 1;
						SourceLocation END1 = END.getLocWithOffset(offset);
						TheRewriter.InsertText(END1, "\n}", true, true); // if {a++}

						TheRewriter.InsertText(Else->getBeginLoc(), "{\n", true, true); // else {a++
						END = Else->getEndLoc();
						offset = Lexer::MeasureTokenLength(END,
														   TheRewriter.getSourceMgr(),
														   TheRewriter.getLangOpts()) +
								 1;
						END1 = END.getLocWithOffset(offset);
						TheRewriter.InsertText(END1, "\n}", true, true);
					}
					else
					{
						TheRewriter.InsertText(Then->getBeginLoc(), "{\n", true, true); // if {a++
						SourceLocation END = Then->getEndLoc();
						int offset = Lexer::MeasureTokenLength(END,
															   TheRewriter.getSourceMgr(),
															   TheRewriter.getLangOpts()) +
									 1;
						SourceLocation END1 = END.getLocWithOffset(offset);
						TheRewriter.InsertText(END1, "\n}", true, true); // if {a++}
					}
				}
				else
				{
					// if a++ |-> if {a++} else {}
					TheRewriter.InsertText(Then->getBeginLoc(), "{\n", true, true);
					SourceLocation END = Then->getEndLoc();
					int offset = Lexer::MeasureTokenLength(END,
														   TheRewriter.getSourceMgr(),
														   TheRewriter.getLangOpts()) +
								 1;
					SourceLocation END1 = END.getLocWithOffset(offset);
					TheRewriter.InsertText(END1, "\n}else{}\n", true, true);
				}
			}
		}

		// Only care about While statements
		if (isa<WhileStmt>(s))
		{
			WhileStmt *WhileStatement = cast<WhileStmt>(s);
			Stmt *WhileBody = WhileStatement->getBody();
			if (!isa<CompoundStmt>(WhileBody))
			{
				TheRewriter.InsertText(WhileBody->getBeginLoc(), "{\n", true, true);
				SourceLocation END = WhileBody->getEndLoc();
				int offset = Lexer::MeasureTokenLength(END, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts()) + 1;
				SourceLocation END1 = END.getLocWithOffset(offset);
				TheRewriter.InsertText(END1, "\n}", true, true);
			}
		}

		// Only care about While statements
		if (isa<ForStmt>(s))
		{
			// 1. For.Init,initial condition
			// 2. For.Inc
			// 2.1. ��ȡFor.Inc�ı�
			// 2.2. ����Ǹ��ϣ���ĩβ����For.Inc
			// 2.3. �����һ�䣬���� {**;For.Inc;}
			// 3. ɾ�� For.Init
			// 4. ɾ�� For.Inc
			// 5. ��for�ĳ�while
			// 6. ɾȥ For.Cond ĩβ����;
			ForStmt *ForStatement = cast<ForStmt>(s);
			Stmt *ForInit = ForStatement->getInit();
			Expr *ForCondition = ForStatement->getCond();
			Expr *ForInc = ForStatement->getInc();
			Stmt *ForBody = ForStatement->getBody();

			// 1. For.Init
			if (isa<BinaryOperator>(ForInit)) // for(i=1;..) |->i=1; for(..)
			{
				std::string OperatorString = BinaryOperatorAsString(ForInit);
				std::stringstream SSBefore;

				SSBefore << OperatorString << ";\n";
				TheRewriter.InsertText(ForStatement->getBeginLoc(), SSBefore.str(), true);
			}

			// 2.1.
			std::string ForIncString = UnaryOperatorAsString(ForInc);
			// 2.2.
			if (isa<CompoundStmt>(ForBody))
			{
				CompoundStmt *CS = cast<CompoundStmt>(ForBody);
				Stmt *endStmt = CS->body_back();
				if (UnaryOperator *UO = dyn_cast<UnaryOperator>(endStmt))
				{
					SourceLocation bodyEndLoc = UO->getSourceRange().getEnd();
					int offset = Lexer::MeasureTokenLength(bodyEndLoc, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts()) + 1;
					SourceLocation EndLoc = bodyEndLoc.getLocWithOffset(offset);
					TheRewriter.InsertText(EndLoc, ForIncString, true, true);
				}
			}
			// 2.3.  {**;For.Inc;}
			else
			{
				TheRewriter.InsertText(ForBody->getBeginLoc(), "{\n", true, true);
				SourceLocation END = ForBody->getEndLoc();
				int offset = Lexer::MeasureTokenLength(END, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts()) + 1;
				SourceLocation END1 = END.getLocWithOffset(offset);
				TheRewriter.InsertText(END1, ForIncString, true, true);
				TheRewriter.InsertTextAfter(END1 , "}");
			}

			// 3.  For.Init
			SourceRange InitRange = ForInit->getSourceRange();
			SourceLocation InitEnd = ForInit->getEndLoc();
			int offset = Lexer::MeasureTokenLength(InitEnd, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts()) + 1;
			InitRange.setEnd(InitEnd.getLocWithOffset(offset));
			TheRewriter.RemoveText(InitRange);

			// 4. For.Inc
			SourceRange IncRange = ForInc->getSourceRange();
			TheRewriter.RemoveText(IncRange);

			// 5. while
			SourceLocation ForBegin = ForStatement->getBeginLoc();
			TheRewriter.ReplaceText(ForBegin, 3, StringRef("while"));

			// 6. ɾȥ For.Cond;
			SourceLocation CondEnd = ForCondition->getEndLoc();
			offset = Lexer::MeasureTokenLength(CondEnd, TheRewriter.getSourceMgr(), TheRewriter.getLangOpts()) + 1;
			SourceRange CondRange = SourceRange(CondEnd, CondEnd.getLocWithOffset(offset));
			if (isa<BinaryOperator>(ForCondition))
			{
				BinaryOperator *BO = cast<BinaryOperator>(ForCondition);
				IntegerLiteral *IL = dyn_cast<IntegerLiteral>(BO->getRHS());
				uint64_t longNumber = IL->getValue().getLimitedValue();
				std::string numberStr = to_string(longNumber);
				// std::string str_cond = BinaryOperatorAsString(BO);
				// llvm::errs() << str_cond;

				TheRewriter.ReplaceText(CondRange, numberStr);
			}
		} //forstmt

		//do_while -> while
		if (isa<DoStmt>(s))
		{
			DoStmt *doStatement = cast<DoStmt>(s);
			SourceRange sourceRange;

			//copy body
			sourceRange.setBegin(doStatement->getBody()->getBeginLoc().getLocWithOffset(1));
			sourceRange.setEnd(doStatement->getBody()->getEndLoc().getLocWithOffset(-1));
			TheRewriter.ReplaceText(doStatement->getBeginLoc().getLocWithOffset(-1), sourceRange);

			Stmt *dobody = doStatement->getBody();
			Expr *whilecond = doStatement->getCond();
			BinaryOperator *BO = cast<BinaryOperator>(whilecond);
			//cond:expr*->string
			std::string str_cond = BinaryOperatorAsString(BO);
			llvm::errs() << str_cond;

			sourceRange.setBegin(doStatement->getWhileLoc());
			sourceRange.setEnd(doStatement->getEndLoc());
			//delete "do"
			TheRewriter.RemoveText(doStatement->getBeginLoc(), 2);
			TheRewriter.InsertTextBefore(doStatement->getBeginLoc(), "while(");
			TheRewriter.InsertTextAfter(doStatement->getBeginLoc(), str_cond);
			TheRewriter.InsertTextAfter(doStatement->getBeginLoc(), ")");
			TheRewriter.RemoveText(sourceRange);
			//delete ;
			TheRewriter.RemoveText(doStatement->getEndLoc().getLocWithOffset(1), 1);
		}

		Switchtoif(s);

		return true;
	} //bool VisitStmt(Stmt *s)

	std::string BinaryOperatorAsString(Stmt *st)
	{
		StringRef lString;
		StringRef rString;
		StringRef opString;
		//BinaryOperator �Ƕ�Ŀ�����
		if (isa<BinaryOperator>(st))
		{

			BinaryOperator *bo = cast<BinaryOperator>(st);
			Expr *lhs = bo->getLHS()->IgnoreParens(); // ������ʽת��
			Expr *rhs = bo->getRHS()->IgnoreParens();
			opString = bo->getOpcodeStr();
			if (isa<DeclRefExpr>(lhs))
			{
				DeclRefExpr *DR = cast<DeclRefExpr>(lhs);

				VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl());
				lString = VD->getName();
				// llvm::errs()<<lString;
			}
			if (isa<ImplicitCastExpr>(lhs))
			{
				ImplicitCastExpr *ICE = cast<ImplicitCastExpr>(lhs);
				Expr *SubExpr = ICE->getSubExpr();
				DeclRefExpr *DR_SubExpr = cast<DeclRefExpr>(SubExpr);
				VarDecl *VD = dyn_cast<VarDecl>(DR_SubExpr->getDecl());
				lString = VD->getName();
			}
			if (isa<IntegerLiteral>(rhs))
			{
				IntegerLiteral *IL = cast<IntegerLiteral>(rhs);
				uint64_t longNumber = IL->getValue().getLimitedValue();
				std::string numberStr = to_string(longNumber);
				rString = StringRef(numberStr);
			}
		}
		return lString.str() + opString.str() + rString.str();
	}

	std::string UnaryOperatorAsString(Expr *E)
	{
		std::stringstream SString;
		if (isa<UnaryOperator>(E)) // i++;
		{
			UnaryOperator *UO = cast<UnaryOperator>(E);
			StringRef typeNmae = UnaryOperator::getOpcodeStr(UO->getOpcode()); // ++
			Expr *subExpr = UO->getSubExpr();
			if (DeclRefExpr *DR = dyn_cast<DeclRefExpr>(subExpr))
			{
				VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl()); // i
				SString << "\n"
						<< VD->getName().str() << typeNmae.str() << ";\n";
			}
		}
		else
		{
			llvm::errs() << "\nThis is not node UnaryOperator\n";
		}
		return SString.str();
	}

	bool Switchtoif(Stmt *s)
	{
		if (isa<SwitchStmt>(s))
		{
			SwitchStmt *switchStatement = cast<SwitchStmt>(s);
			SourceRange sourceRange;

			//make a string from the switch condition
			Expr *condExpr = switchStatement->getCond();
			bool invalid;
			std::string condition = Lexer::getSourceText(CharSourceRange(condExpr->getSourceRange(), true),
														 TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid)
										.str();

			bool isVariable = find_if(condition.begin(), condition.end(),
									  [](char c) { return !(isalnum(c)); }) == condition.end();
			if (!isVariable)
			{
				condition.insert(0, 1, '(').append(")");
			}
			condition.append(" == ");

			SwitchCase *caseList = switchStatement->getSwitchCaseList();
			//checking if the switch has a default case
			bool hasDefault = false;
			sourceRange.setBegin(caseList->getKeywordLoc());
			sourceRange.setEnd(caseList->getKeywordLoc().getLocWithOffset(6));
			std::string lastCase = Lexer::getSourceText(CharSourceRange(sourceRange, true),
														TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid)
									   .str();
			if (lastCase.compare("default") == 0)
			{
				hasDefault = true;
			}

			//creating a vector of statement strings
			//Note: starts from bottom to top case
			Stmt *body = switchStatement->getBody();
			sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
			sourceRange.setEnd(body->getEndLoc().getLocWithOffset(-1));
			std::vector<std::string> caseStatements;

			caseStatements.push_back(Lexer::getSourceText(CharSourceRange(sourceRange, true),
														  TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid)
										 .str());

			while (caseList->getNextSwitchCase() != nullptr)
			{
				sourceRange.setBegin(caseList->getNextSwitchCase()->getColonLoc().getLocWithOffset(1));
				sourceRange.setEnd(caseList->getKeywordLoc().getLocWithOffset(-1));
				caseStatements.emplace_back(Lexer::getSourceText(CharSourceRange(sourceRange, true),
																 TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid)
												.str());
				caseList = caseList->getNextSwitchCase();
			}

			//Note: for fall throughs we need to move from right to left
			//to respect the case order
			int i;
			for (i = caseStatements.size() - 1; i >= 0; i--)
			{
				if (!hasBreak(caseStatements[i]))
				{
					for (int j = i - 1; j >= 0; j--)
					{
						if (hasBreak(caseStatements[j]))
						{
							caseStatements[i].append(caseStatements[j]);
							eraseBreak(caseStatements[i]);
							break;
						}
						caseStatements[i].append(caseStatements[j]);
					}
				}
				else
					eraseBreak(caseStatements[i]);
			}

			//adding if-elseif-else
			//reseting back after making the vector
			caseList = switchStatement->getSwitchCaseList();
			//Note: we are moving through the vector from
			//left to right when falling through
			bool defaultCaseRewritten = false;
			int numberOfStmts = caseStatements.size();
			i = 0;
			SourceLocation nextCaseLoc = body->getEndLoc();

			while (caseList != nullptr && i < numberOfStmts)
			{
				if (hasDefault == true && defaultCaseRewritten == false)
				{
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), "}else if{");
					TheRewriter.ReplaceText(caseList->getKeywordLoc(), 7, "");
					TheRewriter.ReplaceText(caseList->getColonLoc(), 1, "");

					sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
					sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
					//indentCaseStmt(caseStatements[i]);
					TheRewriter.ReplaceText(sourceRange, caseStatements[i]);

					defaultCaseRewritten = true;
					nextCaseLoc = caseList->getKeywordLoc();
					caseList = caseList->getNextSwitchCase();
					i++;
					continue;
				}
				if (caseList->getNextSwitchCase() == nullptr)
				{
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), std::string{"if("}.append(condition));
				}
				else
				{
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), std::string{"}else if("}.append(condition));
				}
				TheRewriter.ReplaceText(caseList->getKeywordLoc(), 4, "");
				TheRewriter.ReplaceText(caseList->getColonLoc(), 1, "){\n");

				//indentCaseStmt(caseStatements[i]);
				sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
				sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
				TheRewriter.ReplaceText(sourceRange, caseStatements[i]);

				nextCaseLoc = caseList->getKeywordLoc();
				caseList = caseList->getNextSwitchCase();
				i++;
			}
			//removing space from switch to the first case
			sourceRange.setBegin(switchStatement->getSwitchLoc());
			sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
			TheRewriter.RemoveText(sourceRange);

		} //if (isa<switchstmt>(s))
		return true;
	} //bool Switchtoif(Stmt *s)

	bool VisitFunctionDecl(FunctionDecl *f)
	{
		// Function name
		DeclarationName DeclName = f->getNameInfo().getName();
		std::string FuncName = DeclName.getAsString();
		if (FuncName == "implicit used printf")
		{
			TheRewriter.InsertTextAfter(f->getEndLoc(), "//this is print");
		}
		// Only function definitions (with bodies), not declarations.
		if (f->hasBody())
		{

			Stmt *FuncBody = f->getBody();

			// Type name as string
			QualType QT = f->getReturnType();
			std::string TypeStr = QT.getAsString();

			// Function name
			DeclarationName DeclName = f->getNameInfo().getName();
			std::string FuncName = DeclName.getAsString();

			// param
			int param_num = f->getNumParams(); // clang/AST/Decl.h��
			std::string func_param;
			//for (FunctionDecl::param_iterator fit = f->param_begin(); fit != f->param_end(); fit++)
			for (int i = 0; i < param_num; i++)
			{
				ParmVarDecl *ptemp = f->getParamDecl(i);
				func_param += " | ";

				//func_param += fit->getOriginalType().getAsString();

				func_param += ptemp->getOriginalType().getAsString();
			}

			// Add comment before
			std::stringstream SSBefore;
			SSBefore << "// Begin function " << FuncName << ", returning " << TypeStr << ", param num: " << param_num << ", type: " << func_param
					 << "\n";

			SourceLocation ST = f->getSourceRange().getBegin();

			TheRewriter.InsertText(ST, SSBefore.str(), true, true);

			// And after
			std::stringstream SSAfter;
			SSAfter << "\n// End function " << FuncName << "\n";
			ST = FuncBody->getEndLoc().getLocWithOffset(1);

			TheRewriter.InsertText(ST, SSAfter.str(), true, true);
		}
		return true;
	}

private:
	Rewriter &TheRewriter;
	CompilerInstance &TheCompiler;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer
{
private:
	MyASTVisitor Visitor;

public:
	MyASTConsumer(Rewriter &R, CompilerInstance &Compiler) : Visitor(R, Compiler) {}

	// Override the method that gets called for each parsed top-level

	bool HandleTopLevelDecl(DeclGroupRef DR) override
	{
		for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
		{
			// Traverse the declaration using our AST visitor.

			Visitor.TraverseDecl(*b);

			// (*b)->dump();
		}
		return true;
	}
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction
{
public:
	MyFrontendAction() {}
	void EndSourceFileAction() override
	{
		SourceManager &SM = TheRewriter.getSourceMgr();
		llvm::errs() << "** EndSourceFileAction for: "
					 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
		// Now emit the rewritten buffer.
		//TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());

		// Now emit the rewritten buffer.
		//  TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs()); --> this will output to screen as what you got.
		std::error_code error_code;
		llvm::raw_fd_ostream outFile("output.cpp", error_code, llvm::sys::fs::F_None);
		TheRewriter.getEditBuffer(SM.getMainFileID()).write(outFile); // --> this will write the result to outFile
		outFile.close();
	}

	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
												   StringRef file) override
	{
		llvm::errs() << "** Creating AST consumer for: " << file << "\n";
		TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		return std::make_unique<MyASTConsumer>(TheRewriter, CI);
	}

private:
	Rewriter TheRewriter;
};

int Addline()
{

	return 0;
}

int main(int argc, const char **argv)
{
	CommonOptionsParser op(argc, argv, ToolingSampleCategory);
	ClangTool Tool(op.getCompilations(), op.getSourcePathList());
	int n = 1;
	char buf[1024];
	FILE *fp, *fp1;
	/* 打开文件，文件名必须大写 */
    fp= fopen("output.cpp", "r");
    if (!fp) {
        printf("No 'output.cpp' found.\n");
        return -1;
    }
 
    fp1= fopen("outputs.cpp", "w");
    if (!fp) {
        printf("No 'loop1.cpp' found.\n");
        return -1;
    }
 
    /* 逐行读取 */
    while (fgets(buf, 1024, fp) > 0)
        fprintf(fp1, "#line %d\n%s  ", n++, buf);
 
    fclose(fp);
    fclose(fp1);
	
	return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
