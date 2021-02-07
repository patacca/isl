#ifndef ISL_INTERFACE_GENERATOR_H
#define ISL_INTERFACE_GENERATOR_H

#include <map>
#include <set>
#include <string>

#include <clang/AST/Decl.h>

using namespace std;
using namespace clang;

/* isl_class collects all constructors and methods for an isl "class".
 * "name" is the name of the class.
 * If this object describes a subclass of a C type, then
 * "subclass_name" is the name of that subclass and "superclass_name"
 * is the name of the immediate superclass of that subclass.  Otherwise,
 * "subclass_name" is equal to "name" and "superclass_name" is undefined.
 * "type" is the declaration that introduces the type.
 * "persistent_callbacks" contains the set of functions that
 * set a persistent callback.
 * "methods" contains the set of methods, grouped by method name.
 * "fn_to_str" is a reference to the *_to_str method of this class, if any.
 * "fn_copy" is a reference to the *_copy method of this class, if any.
 * "fn_free" is a reference to the *_free method of this class, if any.
 * "fn_type" is a reference to a function that described subclasses, if any.
 * If "fn_type" is set, then "type_subclasses" maps the values returned
 * by that function to the names of the corresponding subclasses.
 */
struct isl_class {
	string name;
	string superclass_name;
	string subclass_name;
	RecordDecl *type;
	set<FunctionDecl *> constructors;
	set<FunctionDecl *> persistent_callbacks;
	map<string, set<FunctionDecl *> > methods;
	map<int, string> type_subclasses;
	FunctionDecl *fn_type;
	FunctionDecl *fn_to_str;
	FunctionDecl *fn_copy;
	FunctionDecl *fn_dump;
	FunctionDecl *fn_free;

	/* Is this class a subclass based on a type function? */
	bool is_type_subclass() const { return name != subclass_name; }
	/* Return name of "fd" without type suffix, if any. */
	static string name_without_type_suffix(FunctionDecl *fd);
	/* Extract the method name corresponding to "fd". */
	string method_name(FunctionDecl *fd) const {
		string m_name = name_without_type_suffix(fd);
		return m_name.substr(subclass_name.length() + 1);
	}
	/* The prefix of any method that may set a (persistent) callback. */
	static const char *set_callback_prefix;
	/* Given a function that sets a persistent callback,
	 * return the name of the callback.
	 */
	string persistent_callback_name(FunctionDecl *fd) const {
		return method_name(fd).substr(strlen(set_callback_prefix));
	}
	/* Does this class have any functions that set a persistent callback?
	 */
	bool has_persistent_callbacks() const {
		return persistent_callbacks.size() != 0;
	}
};

/* Base class for interface generators.
 */
class generator {
protected:
	SourceManager &SM;
	map<string,isl_class> classes;
	map<string, FunctionDecl *> functions_by_name;

public:
	generator(SourceManager &SM, set<RecordDecl *> &exported_types,
		set<FunctionDecl *> exported_functions,
		set<FunctionDecl *> functions);

	virtual void generate() = 0;
	virtual ~generator() {};

protected:
	void add_subclass(RecordDecl *decl, const string &name,
		const string &sub_name);
	void add_class(RecordDecl *decl);
	void add_type_subclasses(FunctionDecl *method);
	isl_class *method2class(FunctionDecl *fd);
	bool callback_takes_argument(ParmVarDecl *param, int pos);
	FunctionDecl *find_by_name(const string &name, bool required);
public:
	static void die(const char *msg) __attribute__((noreturn));
	static void die(string msg) __attribute__((noreturn));
	static vector<string> find_superclasses(Decl *decl);
	static bool is_subclass(FunctionDecl *decl);
	static bool is_overload(Decl *decl);
	static bool is_constructor(Decl *decl);
	static bool takes(Decl *decl);
	static bool keeps(Decl *decl);
	static bool gives(Decl *decl);
	static bool is_isl_ctx(QualType type);
	static bool first_arg_is_isl_ctx(FunctionDecl *fd);
	static bool is_isl_type(QualType type);
	static bool is_isl_neg_error(QualType type);
	static bool is_isl_bool(QualType type);
	static bool is_isl_stat(QualType type);
	static bool is_isl_size(QualType type);
	static bool is_long(QualType type);
	static bool is_callback(QualType type);
	static bool is_string(QualType type);
	static bool is_static(const isl_class &clazz, FunctionDecl *method);
	static bool is_mutator(const isl_class &clazz, FunctionDecl *fd);
	static string extract_type(QualType type);
	static const FunctionProtoType *extract_prototype(QualType type);
	static ParmVarDecl *persistent_callback_arg(FunctionDecl *fd);
};

#endif /* ISL_INTERFACE_GENERATOR_H */
