# Tools Boost

给纯python项目[1F1E33-float32/Tools](https://github.com/1F1E33-float32/Tools)提供加速

## 使用方法

### CAT System Crypto

```python
from tools_boost.cat_system_crypto import MT, Blowfish

mt = MT(12345)
random_number = mt.genrand()

key = b"your_key_here"
bf = Blowfish(key)
plaintext = bf.decrypt(ciphertext)
```

### HCA Decryptor

```python
from tools_boost.hca_decryptor import decrypt

with open('encrypted.hca', 'rb') as f:
    encrypted_data = f.read()

decrypted_data = decrypt(encrypted_data, mainkey=0x12345678, subkey=None)

with open('decrypted.hca', 'wb') as f:
    f.write(decrypted_data)
```

## 兼容性
只测试过以下环境：
- `Windows 10/11 x86_64` + `Visual Studio 2022`
- `Windows Subsystem for Linux` + `gcc`