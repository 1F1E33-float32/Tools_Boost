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

### XPCM Extractor

```python
from tools_boost.xpcm_extractor import xpcm_to_pcm
import struct

with open('audio.xpcm', 'rb') as f:
    xpcm_data = f.read()

pcm_data, meta = xpcm_to_pcm(xpcm_data)

def write_wav(pcm_data, sample_rate, channels, bits_per_sample, output_path):
    byte_rate = sample_rate * channels * (bits_per_sample // 8)
    block_align = channels * (bits_per_sample // 8)
    data_size = len(pcm_data)
    
    with open(output_path, 'wb') as f:
        # RIFF header
        f.write(struct.pack('<4sI4s', b'RIFF', 36 + data_size, b'WAVE'))
        # fmt chunk
        f.write(struct.pack('<4sIHHIIHH', b'fmt ', 16, 1, channels, sample_rate, byte_rate, block_align, bits_per_sample))
        # data chunk
        f.write(struct.pack('<4sI', b'data', data_size))
        f.write(pcm_data)

write_wav(pcm_data, meta['sample_rate'], meta['channels'], meta['bits_per_sample'], 'output.wav')
```

## 兼容性
只测试过以下环境：
- `Windows 10/11 x86_64` + `Visual Studio 2022`
- `Windows Subsystem for Linux` + `gcc`

## 参考
- https://github.com/Dir-A/CMakeModules
- https://github.com/vgmstream/vgmstream
- https://github.com/nanami5270/GARbro-Mod