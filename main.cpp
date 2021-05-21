#include "IntermediateCode.h"
#include "ObjectCodeGenerator.h"
#include "parser.h"
#include "lextual.h"

int main() {
	Lextual a;
	a.getResult("D:\\programing\\visual studio code\\C++\\compile\\resources\\test.txt", 
				"D:\\programing\\visual studio code\\C++\\compile\\resources\\lexresult.txt");
	ofstream out("D:\\programing\\visual studio code\\C++\\compile\\resources\\result.txt");
	Parser parser("D:\\programing\\visual studio code\\C++\\compile\\resources\\produtions.txt");
	parser.analyse_e("D:\\programing\\visual studio code\\C++\\compile\\resources\\lexresult.txt", out);
    IntermediateCode code = parser.get_code();
    code.divide_block(parser.get_funcenter());

	ObjectCodeGenerator objectCodeGenerator;
	objectCodeGenerator.block_addinfo(code.get_func_blocks());
	objectCodeGenerator.output_blocks(cout);
	objectCodeGenerator.generate_codes();
	objectCodeGenerator.output_object_codes("D:\\programing\\visual studio code\\C++\\compile\\resources\\program.asm");

	return 0;
}