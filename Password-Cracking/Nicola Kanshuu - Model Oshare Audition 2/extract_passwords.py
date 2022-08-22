# requirements: python 3

# how to get the data:
# get the decompressed code.bin for the game (i tested this with the cart edition)
# extract 430 bytes starting from offset 0x154F90 until 0x15513D
# save the data to a file called jp_pwdata.bin
# run this script in the same folder where you saved the file
# it should print the passwords to the terminal

with open("jp_pwdata.bin", "rb") as f:
	rawdata = f.read()

DECODE_TABLE = {
	0x2B: 'わ',
	0x26: 'ら',
	0x23: 'や',
	0x1E: 'ま',
	0x19: 'は',
	0x14: 'な',
	0x0F: 'た',
	0x0A: 'さ',
	0x05: 'か',
	0x00: 'あ',
	0x2C: 'を',
	0x27: 'リ',
	0x24: 'ゆ',
	0x1F: 'み',
	0x1A: 'ひ',
	0x15: 'に',
	0x10: 'ち',
	0x0B: 'し',
	0x06: 'き',
	0x01: 'い',
	0x39: 'ー',
	0x2D: 'ん',
	0x28: 'る',
	0x25: 'よ',
	0x20: 'む',
	0x1B: 'ふ',
	0x16: 'ぬ',
	0x11: 'つ',
	0x0C: 'す',
	0x07: 'く',
	0x02: 'う',
	0xAF: '　',
	0x2E: '。',
	0x29: 'れ',
	0xE4: '！',
	0x21: 'め',
	0x1C: 'へ',
	0x17: 'ね',
	0x12: 'て',
	0x0D: 'せ',
	0x08: 'け',
	0x03: 'え',
	0x2F: '、',
	0x2A: 'ろ',
	0xE5: '？',
	0x22: 'も',
	0x1D: 'ほ',
	0x18: 'の',
	0x13: 'と',
	0x0E: 'そ',
	0x09: 'こ',
	0x04: 'お'
}

for i in range(43):
	curdata = rawdata[i * 10:(i * 10) + 10]
	# garbage = curdata[0:4]
	pw = curdata[4:10]
	
	pwstr = ""

	for byte in pw:
		pwstr += DECODE_TABLE[byte]

	print(f"{pwstr}\n")