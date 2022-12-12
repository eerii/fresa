#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <llvm/Support/raw_ostream.h>
using namespace clang;

namespace 
{
    struct ReflectionConsumer : public ASTConsumer {
        //* visitor
        struct Visitor : public clang::RecursiveASTVisitor<Visitor> {
            ReflectionConsumer* parent;
            explicit Visitor(ReflectionConsumer* parent) : parent(parent) {}

            //- visit
        };

        //* constructor
        Visitor visitor;
        explicit ReflectionConsumer() : visitor(this) {}

        //* handle translation unit
        void HandleTranslationUnit(ASTContext &ast_ctx) override {
            /*auto *tu_decl = ast_ctx.getTranslationUnitDecl();

            for (const auto *decl : tu_decl->decls()) {
                const auto *nd = dyn_cast<NamedDecl>(decl);
                if (nd) {`
                    //: skip namespaces outside of fresa
                    auto name = nd->getQualifiedNameAsString();
                    if (not name.starts_with("fresa::"))
                        continue;

                    //: print name
                    llvm::errs() << "[REFL]: " << nd->getDeclKindName() << " " << name << "\n";
                }
            }*/
        }
    };

    struct ReflectionPlugin : public PluginASTAction {
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &instance, llvm::StringRef file) override {
            //: print file name
            llvm::errs() << "[REFL]: Reflecting '" << file << "'\n";
            return std::make_unique<ReflectionConsumer>();
        }

        PluginASTAction::ActionType getActionType() override {
            return AddAfterMainAction;
        }

        bool ParseArgs(const CompilerInstance &instance, const std::vector<std::string> &args) override {
            if (!args.empty() && args[0] == "help")
                PrintHelp(llvm::errs());
            return true;
        }

        void PrintHelp(llvm::raw_ostream &ros) {
            ros << "reflection help\n";
        }
    };
}

static FrontendPluginRegistry::Add<ReflectionPlugin> Register("reflection", "");