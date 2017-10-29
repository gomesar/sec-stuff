'''function Criptografia(Senha: string): string;
var x, y: Integer;
NovaSenha: string;
begin
	for x := 1 to Length(Chave) do
		begin
			NovaSenha := '';
			for y := 1 to Length(Senha) do
				NovaSenha := NovaSenha + chr((Ord(Chave[x]) xor Ord(Senha[y])));
			Senha := NovaSenha;
		end;
	result := Senha;
end; 
'''
import base64

chave = "COISABOA"

# 6,7,8
num_swap = {
	'0':'6',	# 678
	#'1':'1',
	'2':'3',
	'3':'4',
	'4':'7',	# 678
	'5':'8',	# 678
	'6':'9',
	'7':'0',
	'8':'2',
	'9':'5',
	'-':'+'
}

def criptografia(senha):
	nova_senha = ''
	
	for i in range(len(chave)):
		nova_senha = ''
		for j in range(len(senha)):
			nova_senha = nova_senha + chr( ord(chave[i]) ^ ord(senha[j]) )
		senha = nova_senha
	result = senha
	
	return result


def translate(encoded):
	senha = encoded.swapcase()
	senha2 = ''
	for i in range(len(senha)):
		if senha[i] in num_swap.keys():
			#print(num_swap[senha[i]])
			senha2 += num_swap[senha[i]]
		else:
			senha2 += senha[i]
	
	return senha2
			
			
def decrypt(encoded):
	encoded = translate(encoded)
	b64_ok = False
	count = 0
	while (not b64_ok):
		try:
			senha = base64.b64decode(encoded)
			b64_ok = True
		except Exception as e:
			encoded += "="
			count += 1
			if count > 5:
				quit()
		
	nova_senha = ''
	
	for i in range(len(chave)-1, -1, -1):
		nova_senha = ''
		for j in range(len(senha)):
			if isinstance(senha[j], int):
				nova_senha = nova_senha + chr( ord(chave[i]) ^ senha[j] )
			elif isinstance(senha[j], str):
				nova_senha = nova_senha + chr( ord(chave[i]) ^ ord(senha[j]) )
			else:
				print("Error")
		senha = nova_senha
	result = senha
	print(result)
	
	return result
	
def encrypt(text):
	coded = base64.b64encode(criptografia(text).encode('ascii'))
	print(coded)
	
	return coded


def encrypt2(text):
	coded = base64.urlsafe_b64encode(criptografia(text).encode('ascii'))
	print(coded)
	
	return coded
	
texto1 = "[TXTCONFIGS]"
texto2 = "[NOMEDB]trabuc_dotcom"
texto3 = "qeHEsu1sx1rjx1LgBgXSnw6PENLUEhPPnxH7DJv9Aq"
#a = encrypt(texto1)
#b = encrypt(texto2)
#c = decrypt(texto3)



decoded_content = ''
with open('logs.txt', 'r') as log:
	encoded = log.readline()
	while encoded:
		decoded = decrypt(encoded)
	
		decoded_content += decoded + '\n'
		encoded = log.readline()
	
with open('log_decoded.txt', 'w') as dlog:
	dlog.write(decoded_content)
	

