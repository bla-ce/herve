import bcrypt

password = b"hello, sir"

hashed = bcrypt.hashpw(password, b"$2b$12$IKBGTqJ8HuoAbN707WukO.")

print(hashed)
