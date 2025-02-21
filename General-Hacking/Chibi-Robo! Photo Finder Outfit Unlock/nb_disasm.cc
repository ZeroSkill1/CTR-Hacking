#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>

template <typename TCast>
inline TCast cst(void *p) { return reinterpret_cast<TCast>(p); }

template <typename T>
struct OffsetArray {
public:
	T *Get(void *base, int index) {
		return cst<T *>(&cst<uint8_t *>(base)[this->Offsets[index]]);
	}

	uint32_t Count;
	uint32_t Offsets[];
};

template <typename T>
static void load_offset_array(std::vector <T *>& out, void *base, void *array) {
	OffsetArray<T> *arr = cst<OffsetArray<T> *>(array);
	uint8_t *bas = cst<uint8_t *>(base);

	out.reserve(arr->Count);

	for (uint32_t i = 0; i < arr->Count; i++)
		out.push_back(cst<T *>(&bas[arr->Offsets[i]]));
}	

struct FunctionEntry {
	uint32_t code_offset;
	uint16_t signature;
	char name[12];
};

struct ExternFunctionEntry {
	uint32_t code_offset;
	uint32_t library;
	uint16_t signature;
	char name[14];
};

struct VariableEntry {
	uint32_t var_offset;
	uint16_t signature;
	char name[14];
};

struct ScriptHeader {
	char magic[4];
	uint32_t unk;
	uint32_t size;
	uint32_t funcs_offset;
	uint32_t extern_funcs_offset;
	uint32_t externs_offset;
	uint32_t local_variables_offset;
	uint32_t sources_offset;
	char original_name[16];
};

static_assert(sizeof(ScriptHeader) == 0x30);

class Script {
	public:
		int Load(char *filename) {
			struct stat st;
			if (stat(filename, &st) != 0) {
				perror("could not stat file");
				return 0;
			}

			if (st.st_size < sizeof(ScriptHeader)) {
				fprintf(stderr, "script file is too small\n");
				return 0;
			}

			this->ScriptData = (uint8_t *)malloc(st.st_size);

			if (!this->ScriptData) {
				perror("could not allocate memory for file");
				return 0;
			}

			FILE *f = fopen(filename, "rb");

			if (!f) {
				perror("could not open file for binary reading");
				return 0;
			}

			if (fread(this->ScriptData, 1, st.st_size, f) != st.st_size) {
				perror("could not read file");
				fclose(f);
				return 0;
			}

			fclose(f);

			this->Header = cst<ScriptHeader *>(this->ScriptData);

			if (strncmp(this->Header->magic, "N2BC", 4) != 0) {
				fprintf(stderr, "invalid script magic\n");
				return 0;
			}

			if (st.st_size < this->Header->size) {
				fprintf(stderr, "script file is too small\n");
				return 0;
			}

			OffsetArray<ExternFunctionEntry> *extern_defs = cst<OffsetArray<ExternFunctionEntry> *>(&this->ScriptData[this->Header->extern_funcs_offset]);

			load_offset_array(this->Functions, this->ScriptData, cst<OffsetArray<FunctionEntry> *>(&this->ScriptData[this->Header->funcs_offset]));
			load_offset_array(this->Variables, this->ScriptData, cst<OffsetArray<VariableEntry> *>(&this->ScriptData[this->Header->externs_offset]));
			load_offset_array(this->ExternFunctions, this->ScriptData, extern_defs);
			this->SourcesOffsets = cst<uint32_t *>(&this->ScriptData[this->Header->sources_offset]);
			this->LocalVariables = cst<OffsetArray<uint32_t[3]> *>(&this->ScriptData[this->Header->local_variables_offset]);

			/* each source file ptr must be between the last extern function entry's end and offset array of local functions */
			if (this->Header->sources_offset) {
				uint32_t *sources_arr = cst<uint32_t *>(&this->ScriptData[this->Header->sources_offset]);
				while (1) {
					uint32_t sourcefile_ptr = *sources_arr;
					if (!(sourcefile_ptr >= this->Header->sources_offset && sourcefile_ptr < this->Header->externs_offset))
						break;
					this->SourceFileNames.push_back(cst<char *>(&this->ScriptData[sourcefile_ptr]));
					sources_arr++;
				}
			}

			return 1;
		}

		~Script() {
			if (this->ScriptData != nullptr)
				free(this->ScriptData);
		}

		enum class opcode : uint8_t {
			nop            = 0x00,
			line           = 0x01,
			stackalloc     = 0x02,
			push           = 0x03,
			stackfree      = 0x04,
			pop            = 0x05,
			push_multi     = 0x06,
			pop_multi      = 0x07,
			push_prog      = 0x08,
			pop_prog       = 0x09,
			mov_imm        = 0x0A,
			or_imm         = 0x0B,
			adr_flt        = 0x0C,
			adr_str        = 0x0D,
			mov_bool       = 0x0E,
			clr_reg        = 0x0F,
			ary_create     = 0x10,
			mov_reg        = 0x11,
			ldr_var_ref    = 0x12,
			ldr_extvar_ref = 0x13,
			ldr_locvar_ref = 0x14,
			ldr_reg        = 0x15,
			cpy_extvar     = 0x16,
			cpy_locvar     = 0x17,
			ldr_locfn      = 0x18,
			ldr_extfn      = 0x19,
			jump           = 0x1A,
			jumptrue       = 0x1B,
			jumpfalse      = 0x1C,
			call           = 0x1D,
			retn           = 0x1E,
			add            = 0x1F,
			udf_0x20       = 0x20,
			sub            = 0x21,
			udf_0x22       = 0x22,
			mul            = 0x23,
			udf_0x24       = 0x24,
			div            = 0x25,
			udf_0x26       = 0x26,
			mod            = 0x27,
			chk_true       = 0x28,
			chk_eq         = 0x29,
			chk_neq        = 0x2A,
			chk_lt         = 0x2B,
			chk_gt         = 0x2C,
			chk_lteq       = 0x2D,
			chk_gteq       = 0x2E,
			land           = 0x2F,
			lor            = 0x30,
			bflips         = 0x31,
			ands           = 0x32,
			xors           = 0x33,
			orrs           = 0x34,
			nots           = 0x35,
			lsl            = 0x36,
			lsr            = 0x37,
			asr            = 0x38,
			set_var        = 0x39,
			perf_log       = 0x3A,
			function       = 0x3B,
			ary_push       = 0x3D,
			ary_get_ref    = 0x3E,
			ary_get        = 0x3F,
		};

		struct __attribute__((packed)) imm24 {
			int32_t get() {
				int32_t tmp;
				memcpy(&tmp, this->raw, 3);
				return tmp & 0x800000 ? 0xFF000000 | tmp : tmp;
			}
			uint8_t raw[3];
		};

		static_assert(sizeof(imm24) == 3);

		struct __attribute__((packed)) insn {
			opcode op;
			union {
				struct __attribute__((packed)) {
					uint8_t arg0;
					uint8_t arg1;
					uint8_t arg2;
				} ver_a;
				struct __attribute__((packed)) {
					uint8_t arg0;
					uint16_t arg1;
				} ver_b;
				struct __attribute((packed)) {
					imm24 arg0;
				} ver_c;
			} argdata;
		};

		static_assert(sizeof(insn) == 4);

		static std::string get_reg_range(uint16_t range) {
			std::string r = "{";
			for (int i = 0; i < 8; i++) {
				char regname[4] = { 'R', '0' + i, ',' };
				if (range & (1 << (7 - i))) {
					if (i != 7) regname[2] = ',';	
					else regname[2] = '}';
					r += regname;
				}
			}
			return r;
		}

		int DumpDissasemblyOf(FunctionEntry *func) {
			if (std::find(this->Functions.begin(), this->Functions.end(), func) == this->Functions.end()) {
				fprintf(stderr, "could not find functions\n");
				return 0;
			}

			insn *codeptr = cst<insn *>(&this->ScriptData[func->code_offset]);
			uint32_t offset = func->code_offset;

#define offset_print(fmt) printf("[%08X] " fmt, offset)
#define offset_printf(fmt, ...) printf("[%08X] " fmt, offset, __VA_ARGS__)

			while (1) {
				insn cur = *codeptr;
				switch (cur.op) {
					case opcode::nop:
						offset_print("nop\n");
						break;
					case opcode::line:
						offset_printf("line %s:%d\n", cst<char *>(&this->ScriptData[this->SourcesOffsets[cur.argdata.ver_b.arg0]]), cur.argdata.ver_b.arg1);
						break;
					case opcode::stackalloc:
						offset_printf("stackalloc %d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::push:
						offset_printf("push {R%d}\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::stackfree:
						offset_printf("stackfree %d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::pop:
						offset_printf("pop {R%d}\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::push_multi:
						{
							std::string range = get_reg_range(cur.argdata.ver_b.arg1);
							offset_printf("push %s\n", range.c_str());
						}
						break;
					case opcode::pop_multi:
						{
							std::string range = get_reg_range(cur.argdata.ver_b.arg1);
							offset_printf("pop %s\n", range.c_str());
						}
						break;
					case opcode::push_prog:
						offset_printf("push_prog R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::pop_prog:
						offset_print("pop_prog\n");
						break;
					case opcode::mov_imm:
						offset_printf("mov R%d, #0x%X\n", cur.argdata.ver_b.arg0, cur.argdata.ver_b.arg1);
						break;
					case opcode::or_imm:
						offset_printf("or R%d, #0x%X\n", cur.argdata.ver_b.arg0, cur.argdata.ver_b.arg1 << 16);
						break;
					case opcode::adr_flt:
						offset_printf("adr_flt R%d, =%g\n", cur.argdata.ver_b.arg0, *cst<float *>(&this->ScriptData[offset + (signed short)cur.argdata.ver_b.arg1 * 4]));
						break;
					case opcode::adr_str:
						offset_printf("adr_str R%d, ='%s'\n", cur.argdata.ver_b.arg0, cst<char *>(&this->ScriptData[offset + (signed short)cur.argdata.ver_b.arg1 * 4 + 2]));
						break;
					case opcode::mov_bool:
						offset_printf("mov R%d, %s\n", cur.argdata.ver_b.arg0, cur.argdata.ver_b.arg1 ? "true" : "false");
						break;
					case opcode::clr_reg:
						offset_printf("clr R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::ary_create:
						offset_printf("ary_create, R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::mov_reg:
						offset_printf("mov R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1);
						break;
					case opcode::ldr_var_ref:
						offset_printf("ldr_var_ref R%d, [R%d, #%d]\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg2, (signed char)cur.argdata.ver_a.arg1);
						break;
					case opcode::ldr_extvar_ref:
						offset_printf("ldr_extvar_ref R%d, %s\n", cur.argdata.ver_b.arg0, this->Variables[cur.argdata.ver_b.arg1]->name);
						break;
					case opcode::ldr_locvar_ref:
						offset_printf("ldr_locvar_ref R%d, LV%d\n", cur.argdata.ver_b.arg0, cur.argdata.ver_b.arg1);
						break;
					case opcode::ldr_reg:
						offset_printf("ldr R%d, [R%d, #%d]\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg2, (signed char)cur.argdata.ver_a.arg1);
						break;
					case opcode::cpy_extvar:
						offset_printf("cpy_extvar R%d, LV%d\n", cur.argdata.ver_b.arg0, cur.argdata.ver_b.arg1);
						break;
					case opcode::cpy_locvar:
						offset_printf("cpy_locvar R%d, LV%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1);
						break;
					case opcode::ldr_locfn:
						offset_printf("ldr_locfn R%d, ='%s'\n", cur.argdata.ver_b.arg0, this->Functions[cur.argdata.ver_b.arg1]->name);
						break;
					case opcode::ldr_extfn:
						offset_printf("ldr_extfn R%d, ='%s'\n", cur.argdata.ver_b.arg0, this->ExternFunctions[cur.argdata.ver_b.arg1]->name);
						break;
					case opcode::jump:
						offset_printf("jump =0x%X\n", offset + cur.argdata.ver_c.arg0.get() * 4);
						break;
					case opcode::jumptrue:
						offset_printf("jumptrue =0x%X\n", offset + cur.argdata.ver_c.arg0.get() * 4);
						break;
					case opcode::jumpfalse:
						offset_printf("jumpfalse =0x%X\n", offset + cur.argdata.ver_c.arg0.get() * 4);
						break;
					case opcode::call:
						if (cur.argdata.ver_a.arg1) offset_printf("call_loc R%d\n", cur.argdata.ver_a.arg0);
						else                        offset_printf("call R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::retn:
						{
							offset_print("retn\n");
							return 1;
						}
						break;
					case opcode::add:
						offset_printf("add R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::sub:
						offset_printf("sub R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::mul:
						offset_printf("mul R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::div:
						offset_printf("div R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::mod:
						offset_printf("mod R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_true:
						offset_printf("chk_true R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::chk_eq:
						if (cur.argdata.ver_a.arg0 != 0xFF)
							offset_printf("chk_eq R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						else
							offset_printf("chk_eq (none), R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_neq:
						offset_printf("chk_neq R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_lt:
						offset_printf("chk_lt R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_gt:
						offset_printf("chk_gt R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_lteq:
						offset_printf("chk_lteq R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::chk_gteq:
						offset_printf("chk_gteq R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::land:
						offset_printf("land R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::lor:
						offset_printf("lor R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::bflips:
						offset_printf("bflips R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1);
						break;
					case opcode::ands:
						offset_printf("ands R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::xors:
						offset_printf("xors R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::orrs:
						offset_printf("orrs R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::nots:
						offset_printf("nots R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::lsl:
						offset_printf("lsl R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::lsr:
						offset_printf("lsr R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::asr:
						offset_printf("asr R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::set_var:
						offset_printf("set_var R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::function:
						offset_printf("function '%s'\n", cst<FunctionEntry *>(&this->ScriptData[offset + cur.argdata.ver_c.arg0.get() * 4])->name);
						break;
					case opcode::perf_log:
						offset_printf("perf_log R%d\n", cur.argdata.ver_a.arg0);
						break;
					case opcode::ary_push:
						offset_printf("ary_push R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1);
						break;
					case opcode::ary_get_ref:
						offset_printf("ary_get_ref R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					case opcode::ary_get:
						offset_printf("ary_get R%d, R%d, R%d\n", cur.argdata.ver_a.arg0, cur.argdata.ver_a.arg1, cur.argdata.ver_a.arg2);
						break;
					default:
						offset_printf("\033[31mopcode \033[33m%d\033[0m\n", cur.op);
						break;
				}

				++codeptr;
				offset += 4;
			}

			return 1;
		}

		std::vector<FunctionEntry *> Functions;
		std::vector<VariableEntry *> Variables;
		std::vector<ExternFunctionEntry *> ExternFunctions;
		std::vector<char *> SourceFileNames;
		OffsetArray<uint32_t[3]> *LocalVariables;
		uint32_t *SourcesOffsets  = nullptr;
		ScriptHeader *Header = nullptr;

	private:
		uint8_t *ScriptData = nullptr;
};

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <path to .nb>\n", argv[0]);
		return 1;
	}

	Script s;

	if (!s.Load(argv[1])) {
		fprintf(stderr, "script load failure\n");
		return 1;
	}

	printf("script '%s':\n", argv[1]);
	printf("version (presumably): %d\n", s.Header->unk);
	printf("size: %d (0x%X)\n", s.Header->size, s.Header->size);
	printf("functions offset array offset        : 0x%X\n", s.Header->funcs_offset);
	printf("extern functions offset array offset : 0x%X\n", s.Header->extern_funcs_offset);
	printf("global variables offset array offset : 0x%X\n", s.Header->externs_offset);
	printf("local variables offset array offset  : 0x%X\n", s.Header->local_variables_offset);
	printf("number of local variables            : %d\n", s.LocalVariables->Count);
	printf("source file paths array offset       : 0x%X\n", s.Header->sources_offset);
	printf("original script source file name     : %s\n", s.Header->original_name);

	printf("function list:\n");
	for (int i = 0; i < s.Functions.size(); i++)
		printf("\t[%d | 0x%X]: %s\n", i, i, s.Functions[i]->name);

	printf("global variables list:\n");
	for (int i = 0; i < s.Variables.size(); i++)
		printf("\t[%d | 0x%X]: %s\n", i, i, s.Variables[i]->name);

	printf("extern functions list:\n");
	for (int i = 0; i < s.ExternFunctions.size(); i++)
		printf("\t[%d | 0x%X]: %s\n", i, i, s.ExternFunctions[i]->name);

	printf("referenced source files:\n");
	for (int i = 0; i < s.SourceFileNames.size(); i++)
		printf("\t[%d | 0x%X]: %s\n", i, i, s.SourceFileNames[i]);

	for (FunctionEntry *i : s.Functions) {
		printf("script function '%s' @ 0x%X\n", i->name, i->code_offset);
		s.DumpDissasemblyOf(i);
	}


	return 0;
}
