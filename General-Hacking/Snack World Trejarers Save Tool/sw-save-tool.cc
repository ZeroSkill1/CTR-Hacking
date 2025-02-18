#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/ccm.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zlib.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

namespace xorshift {
	/* first 254 primes */
	static const u16 primes_lut[280] = {
	  3u,    5u,    7u,    11u,   13u,   17u,   19u,   23u,   29u,   31u,
	  37u,   41u,   43u,   47u,   53u,   59u,   61u,   67u,   71u,   73u,
	  79u,   83u,   89u,   97u,   101u,  103u,  107u,  109u,  113u,  127u,
	  131u,  137u,  139u,  149u,  151u,  157u,  163u,  167u,  173u,  179u,
	  181u,  191u,  193u,  197u,  199u,  211u,  223u,  227u,  229u,  233u,
	  239u,  241u,  251u,  257u,  263u,  269u,  271u,  277u,  281u,  283u,
	  293u,  307u,  311u,  313u,  317u,  331u,  337u,  347u,  349u,  353u,
	  359u,  367u,  373u,  379u,  383u,  389u,  397u,  401u,  409u,  419u,
	  421u,  431u,  433u,  439u,  443u,  449u,  457u,  461u,  463u,  467u,
	  479u,  487u,  491u,  499u,  503u,  509u,  521u,  523u,  541u,  547u,
	  557u,  563u,  569u,  571u,  577u,  587u,  593u,  599u,  601u,  607u,
	  613u,  617u,  619u,  631u,  641u,  643u,  647u,  653u,  659u,  661u,
	  673u,  677u,  683u,  691u,  701u,  709u,  719u,  727u,  733u,  739u,
	  743u,  751u,  757u,  761u,  769u,  773u,  787u,  797u,  809u,  811u,
	  821u,  823u,  827u,  829u,  839u,  853u,  857u,  859u,  863u,  877u,
	  881u,  883u,  887u,  907u,  911u,  919u,  929u,  937u,  941u,  947u,
	  953u,  967u,  971u,  977u,  983u,  991u,  997u,  1009u, 1013u, 1019u,
	  1021u, 1031u, 1033u, 1039u, 1049u, 1051u, 1061u, 1063u, 1069u, 1087u,
	  1091u, 1093u, 1097u, 1103u, 1109u, 1117u, 1123u, 1129u, 1151u, 1153u,
	  1163u, 1171u, 1181u, 1187u, 1193u, 1201u, 1213u, 1217u, 1223u, 1229u,
	  1231u, 1237u, 1249u, 1259u, 1277u, 1279u, 1283u, 1289u, 1291u, 1297u,
	  1301u, 1303u, 1307u, 1319u, 1321u, 1327u, 1361u, 1367u, 1373u, 1381u,
	  1399u, 1409u, 1423u, 1427u, 1429u, 1433u, 1439u, 1447u, 1451u, 1453u,
	  1459u, 1471u, 1481u, 1483u, 1487u, 1489u, 1493u, 1499u, 1511u, 1523u,
	  1531u, 1543u, 1549u, 1553u, 1559u, 1567u, 1571u, 1579u, 1583u, 1597u,
	  1601u, 1607u, 1609u, 1613u, 1619u, 1621u, 0u,    0u,    0u,    0u,
	  0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u,
	  0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u,    0u
	};

	class algorithm {
	public:
		algorithm(u32 seed = 0) {
			state[0] = 0x6C078966;
			state[1] = 0xDD5254A5;
			state[2] = 0xB9523B81;
			state[3] = 0x3DF95B3;

			if (seed) {
				state[0] = 0x6C078965 * (seed ^ (seed >> 30)) + 1;
				state[1] = 0x6C078965 * (state[0] ^ (state[0] >> 30)) + 2;
				state[2] = 0x6C078965 * (state[1] ^ (state[1] >> 30)) + 3;
			}
		}

		u32 scramble(u32 input = 0) {
			u32 s0 = state[0], s3 = state[3];
			state[0] = state[1];
			state[1] = state[2];
			state[2] = state[3];

			state[3] = s0 ^ (s0 << 11) ^ ((s0 ^ (s0 << 11)) >> 8) ^ s3 ^ (s3 >> 19);

			return input ? state[3] % input : state[3];
		}

	private:
		u32 state[4] = { 0 };
	};

	static algorithm global_seedgen;


	class file_wrapper {
	public:
		struct eof_data {
			u32 crc;
			u32 xorshift_seed;
		};

		static_assert(sizeof(eof_data) == 8, "invalid eof data size");

		file_wrapper(void *file, u32 size) {
			this->file = reinterpret_cast<u8 *>(file);
			this->size = size;
		}

		eof_data *eof_info() { return reinterpret_cast<eof_data *>(&this->file[this->data_size()]); }
		u32 data_size() { return this->size - sizeof(eof_data); }

		void generate_xor_table() {
			u32 *cs_u32 = reinterpret_cast<u32 *>(this->xor_table);
			u8 *cs_u8 = reinterpret_cast<u8 *>(this->xor_table);

			*cs_u32 = this->eof_info()->xorshift_seed;

			while (!this->eof_info()->xorshift_seed)
				*cs_u32 = this->eof_info()->xorshift_seed = xorshift::global_seedgen.scramble(0);

			xorshift::algorithm alg(*cs_u32);

			for (int i = 0; i < 0x100; i += 2 ) {
				cs_u8[i + 4] = i;
				cs_u8[i + 5] = i + 1;
			}

			u8 *streamdata = &cs_u8[4];

			for (int i = 0; i < 0x1000; i++) {
				u32 r0 = alg.scramble(0x10000);
				u32 r2 = r0 << 16;
				u8 scramble_lobyte = r0 & 0xff;
				u8 scramble_hibyte = r2 >> 24;

				if (scramble_lobyte != scramble_hibyte) {
					u8 v13 = streamdata[scramble_hibyte];
					u8 *v14 = &streamdata[streamdata[scramble_lobyte]];
					u8 *v15 = &streamdata[v13];
					v13 = *v14;
					*v14 = *v15;
					*v15 = v13;
				}
			}
		}

		u32 crypt(u32 seed) {
			u8 *stream = this->file;

			this->eof_info()->xorshift_seed = seed;
			this->generate_xor_table();

			u32 processed = 0;
			for (u16 v8 = 0; processed < this->data_size(); processed++) {
				u8 processed_lobyte = (u8)(processed & 0xFF);
				if (!processed_lobyte)
					v8 = primes_lut[this->xor_table[reinterpret_cast<u8 *>(&processed)[1] + 4]];
				*stream++ ^= this->xor_table[(u8)((processed_lobyte + 1) * v8) + 4];
			}

			return reinterpret_cast<u32 *>(this->xor_table)[0];
		}

		int verify_and_decrypt() {
			if (!this->eof_info()->xorshift_seed) {
				fprintf(stderr, "seed cannot be zero for xorshift decryption\n");
				return 0;
			}

			if (crc32(0, this->file, this->data_size()) != this->eof_info()->crc) {
				fprintf(stderr, "CRC32 checksum mismatch\n");
				return 0;
			}

			this->crypt(this->eof_info()->xorshift_seed);

			return 1;
		}

		void encrypt() {
			/* for xorshift encryption, seed is 0, new seed results from crypt */
			this->eof_info()->xorshift_seed = this->crypt(0);
			this->eof_info()->crc = crc32(0, this->file, this->data_size());
		}

		u8 xor_table[0x104] = { 0 };
		u8 *file = nullptr;
		u32 size = 0;
	};
}

void hexprint(void *input, size_t len, bool upper = false, bool space = false) {
	const char *fmt = upper ? "%02X" : "%02x";
	for (size_t i = 0; i < len; i++) {
		printf(fmt, reinterpret_cast<u8 *>(input)[i]);
		if (space && i != len - 1)
			fputc(' ', stdout);
	}

	fputc('\n', stdout);
}

int load_file_into_memory(char *path, void **out_contents, u32 *out_size, u32 extra = 0, u32 load_offset = 0) {
	struct stat st;
	if (stat(path, &st) == -1) {
		perror("stat failed");
		return 0;
	}

	if (st.st_size > 1024 * 1024) {
		fprintf(stderr, "[%s] file is too large (1MiB max)\n", path);
		return 0;
	}

	size_t mem_size = st.st_size + extra;

	if (load_offset + st.st_size > mem_size) {
		fputs("file load offset out of bounds\n", stderr);
		return 0;
	}

	u8 *mem = (u8 *)malloc(mem_size);
	if (!mem) {
		fprintf(stderr, "could not allocate %ld bytes for file\n", mem_size);
		return 0;
	}
	memset(mem, 0, mem_size);

	FILE *f = fopen(path, "rb");
	if (!f || fread(&mem[load_offset], 1, st.st_size, f) != st.st_size) {
		perror("failed opening/reading from file");
		if (f) fclose(f);
		free(mem);
		return 0;
	}

	*out_size = mem_size;
	*out_contents = mem;

	fclose(f);

	return 1;
}

namespace swutil {
	struct crypted_file {
		u8 nonce[0xC];
		u8 _pads[4];
		u8 ccm_tag[0x10];
		u8 file_data[];
	};

	static_assert(offsetof(crypted_file, ccm_tag) == 0x10);
	static_assert(offsetof(crypted_file, file_data) == 0x20);
	static_assert(sizeof(crypted_file) == 0x20);

	int keygen(void *head, u32 head_size, u8 *out_key) {
		if (head_size < 356) {
			fprintf(stderr, "head.sw file data is too small\n");
			return 0;
		}

		u32 *head_32 = reinterpret_cast<u32 *>(head);

		u32 valbase = head_32[4] & ~1;

		u32 seed_part2_offset = 0x20 + 0xA0 * valbase + 36;
		u32 num_noinput_scrambles_offset = 0x20 + 0xA0 * valbase + 0x6C + 6;

		if (num_noinput_scrambles_offset >= head_size || seed_part2_offset >= head_size) {
			fprintf(stderr, "attempted to access a value in head.sw out of range. abort.\n");
			return 0;
		}

		u32 seed_part1 = head_32[3];
		u32 seed_part2 = head_32[seed_part2_offset / 4];
		u8 num_noinput_scrambles = reinterpret_cast<u8 *>(head)[num_noinput_scrambles_offset];

		xorshift::algorithm alg(seed_part1 ^ seed_part2);

		for (int i = 0; i < num_noinput_scrambles; i++)
			alg.scramble();

		for (int i = 0; i < 16; i++)
			out_key[i] = (u8)alg.scramble(0x100);

		return 1;
	}

	int decrypt(void *input, u8 *key, u32 size) {
		crypted_file *cf = reinterpret_cast<crypted_file *>(input);

		mbedtls_ccm_context ctx;
		mbedtls_ccm_init(&ctx);
		mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
		int rv = mbedtls_ccm_auth_decrypt(&ctx, size - sizeof(crypted_file), cf->nonce, sizeof(cf->nonce), NULL, 0, cf->file_data, cf->file_data, cf->ccm_tag, sizeof(cf->ccm_tag));
		mbedtls_ccm_free(&ctx);

		if (rv != 0) {
			if (rv == MBEDTLS_ERR_CCM_AUTH_FAILED)
				fputs("ccm decrypt error: ccm auth failed\n", stderr);
			else {
				char error[256 + 1] = { 0 };
				mbedtls_strerror(rv, error, sizeof(error));
				fprintf(stderr, "ccm decrypt error: %s\n", error);
			}

			return 0;
		}

		return 1;
	}

	int encrypt(void *input, u8 *key, u32 size) {
		crypted_file *cf = reinterpret_cast<crypted_file *>(input);

		mbedtls_entropy_context rctx;
		mbedtls_entropy_init(&rctx);
		mbedtls_entropy_func(&rctx, cf->nonce, sizeof(cf->nonce));
		mbedtls_entropy_free(&rctx);

		mbedtls_ccm_context ctx;
		mbedtls_ccm_init(&ctx);
		mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
		int rv = mbedtls_ccm_encrypt_and_tag(&ctx, size - sizeof(crypted_file), cf->nonce, sizeof(cf->nonce), NULL, 0, cf->file_data, cf->file_data, cf->ccm_tag, sizeof(cf->ccm_tag));
		mbedtls_ccm_free(&ctx);

		if (rv != 0) {
			char error[256 + 1] = { 0 };
			mbedtls_strerror(rv, error, sizeof(error));
			fprintf(stderr, "ccm encrypt error: %s\n", error);
		}

		return 1;
	}
};

char *save_crypted_file(char *src_filename, void *file, u32 size, bool decrypt, bool inplace) {
	char *path = NULL;
	char *dup0 = NULL, *dup1 = NULL;

	if (inplace) {
		path = src_filename;
	}
	else {
		dup0 = strdup(src_filename);
		dup1 = strdup(src_filename);
		if (!dup0 || !dup1) {
			fputs("could not duplicate filename for building output file path\n", stderr);
			if (dup0) free(dup0);
			if (dup1) free(dup1);
			return 0;
		}
		char *dirn = dirname(dup0);
		char *filen = basename(dup1);
		size_t path_size = strlen(dirn) + strlen(filen) + sizeof("_xxx.sw") + 2;
		path = (char *)malloc(path_size);
		path[path_size - 1] = '\0';
		if (!path) {
			fprintf(stderr, "could not allocate memory for %s %s path\n", decrypt ? "decrypted" : "encrypted", src_filename);
			free(dup0);
			free(dup1);
			return NULL;
		}
		snprintf(path, path_size, decrypt ? "%s/dec_%s" : "%s/enc_%s", dirn, filen);
	}

	FILE *o = fopen(path, "wb");
	if (!o || fwrite(file, 1, size, o) != size) {
		perror("opening/writing output file failed");
		if (o) fclose(o);
		if (!inplace) {
			free(dup0);
			free(dup1);
		}
		return NULL;
	}
	fclose(o);

	if (!inplace) {
		free(dup0);
		free(dup1);
	}

	return path;
}

int run_decrypt(int argc, char **argv, bool inplace) {
	void *head = NULL;
	u32 head_size = 0;

	if (!load_file_into_memory(argv[1], &head, &head_size)) {
		fprintf(stderr, "[%s] load failed\n", argv[1]);
		return 1;
	}

	xorshift::file_wrapper xs_headsw(head, head_size);

	if (!xs_headsw.verify_and_decrypt()) {
		fprintf(stderr, "[%s] xorshift decryption/verification failed\n", argv[1]);
		free(head);
		return 1;
	}

	char *npath = save_crypted_file(argv[1], head, xs_headsw.data_size(), true, inplace);
	if (!npath) {
		fprintf(stderr, "[%s] could not save decrypted file\n", argv[1]);
		free(head);
		return 1;
	}

	printf("[%s] saved decrypted version to %s\n", argv[1], npath);
	if (!inplace) free(npath);

	u8 ccm_key[16] = { 0 };

	if (!swutil::keygen(head, xs_headsw.data_size(), ccm_key)) {
		fprintf(stderr, "[%s] could not generate CCM key\n", argv[1]);
		free(head);
		return 1;
	}

 	if (argc > 2) {
		for (int i = 0; i < argc - 2; i++) {
			void *savefile_contents = NULL;
			u32 savefile_size = 0;

			if (!load_file_into_memory(argv[2 + i], &savefile_contents, &savefile_size)) {
				fprintf(stderr, "[%s] load failed\n", argv[2 + i]);
				continue;
			}

			if (!swutil::decrypt(savefile_contents, ccm_key, savefile_size)) {
				fprintf(stderr, "[%s] aes decrypt failed\n", argv[2 + i]);
				free(savefile_contents);
				continue;
			}

			savefile_size -= sizeof(swutil::crypted_file);

			xorshift::file_wrapper xs_save(reinterpret_cast<swutil::crypted_file *>(savefile_contents)->file_data, savefile_size);

			if (!xs_save.verify_and_decrypt()) {
				fprintf(stderr, "[%s] xorshift decryption/verification failed\n", argv[2 + i]);
				free(savefile_contents);
				continue;
			}

			char *nspath = save_crypted_file(argv[2 + i], xs_save.file, xs_save.data_size(), true, inplace);

			if (!nspath) {
				fprintf(stderr, "[%s] save decrypted file failed\n", argv[2 + i]);
				free(savefile_contents);
				continue;
			}

			printf("[%s] saved decrypted version to %s\n", argv[2 + i], nspath);

			if (!inplace) free(nspath);
			free(savefile_contents);
		}
	}

	free(head);
	return 0;
}

int run_encrypt(int argc, char **argv, bool inplace) {
	static const u32 generic_reencrypt_extra_size = sizeof(xorshift::file_wrapper::eof_data);
	static const u32 reencrypt_save_extra_size = generic_reencrypt_extra_size + sizeof(swutil::crypted_file);
	static_assert(reencrypt_save_extra_size == 40, "invalid save reencrypt data size");

	void *head = NULL;
	u32 head_size = 0;

	if (!load_file_into_memory(argv[1], &head, &head_size, generic_reencrypt_extra_size)) {
		fprintf(stderr, "[%s] load failed\n", argv[1]);
		return 1;
	}

	xorshift::file_wrapper xs_headsw(head, head_size);
	u8 ccm_key[16] = { 0 };

	if (!swutil::keygen(head, head_size, ccm_key)) {
		fprintf(stderr, "[%s] could not generate CCM key\n", argv[1]);
		free(head);
		return 1;
	}

	xs_headsw.encrypt();

	char *npath = save_crypted_file(argv[1], head, xs_headsw.size, false, inplace);

	if (!npath) {
		fprintf(stderr, "[%s] could not save re-encrypted file\n", argv[1]);
		free(head);
		return 1;
	}

	printf("[%s] saved encrypted version to %s\n", argv[1], npath);
	if (!inplace) free(npath);

	if (argc > 2) {
		for (int i = 0; i < argc - 2; i++) {
			void *savefile_contents = NULL;
			u32 savefile_size = 0;

			if (!load_file_into_memory(argv[2 + i], &savefile_contents, &savefile_size, reencrypt_save_extra_size, offsetof(swutil::crypted_file, file_data))) {
				fprintf(stderr, "[%s] load failed\n", argv[2 + i]);
				continue;
			}

			swutil::crypted_file *savefile = reinterpret_cast<swutil::crypted_file *>(savefile_contents);
			xorshift::file_wrapper xs_savefile(savefile->file_data, savefile_size - sizeof(swutil::crypted_file));

			xs_savefile.encrypt();

			if (!swutil::encrypt(savefile, ccm_key, savefile_size)) {
				fprintf(stderr, "[%s] aes encrypt failed\n", argv[2 + i]);
				free(savefile_contents);
				continue;
			}

			char *nspath = save_crypted_file(argv[2 + i], savefile_contents, savefile_size, false, inplace);

			if (!nspath) {
				fprintf(stderr, "[%s] save reencrypted file failed\n", argv[2 + i]);
				free(savefile_contents);
				continue;
			}

			printf("[%s] saved encrypted version to %s\n", argv[2 + i], nspath);

			if (!inplace) free(nspath);
			free(savefile_contents);
		}
	}

	free(head);
	return 0;
}

#define countof(x) (sizeof(x) / sizeof(x[0]))


static const char *progname = "sw-save-tool";

void print_usage() {
	fprintf(stderr, "usage: %s encrypt/decrypt inplace/separate <path to head.sw> <path to save file>\n", progname);
	fprintf(stderr, "example: %s decrypt inplace head.sw game1.sw game2.sw\n", progname);
}

int main(int argc, char **argv) {
	if (argc < 4) {
		if (argc) progname = argv[0];
		print_usage();
		return 1;
	}

	struct {
		const char *cmd;
		int (* cb)(int, char **, bool);
	} actions[2] = {
		{ "decrypt", &run_decrypt },
		{ "encrypt", &run_encrypt },
	};

	bool inplace = false;

	if (strncasecmp(argv[2], "inplace", sizeof("inplace") - 1) == 0)
		inplace = true;
	else if (strncasecmp(argv[2], "separate", sizeof("separate") - 1) == 0)
		inplace = false;
	else {
		print_usage();
		return 1;
	}

	for (int i = 0; i < countof(actions); i++) {
		if (strncasecmp(argv[1], actions[i].cmd, sizeof("xxcrypt") - 1) == 0)
			return actions[i].cb(argc - 2, argv + 2, inplace);
	}

	print_usage();
	return 1;
}

#undef countof
