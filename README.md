# Tools Boost

Accelerated native module for the pure-Python project "Tools":
https://github.com/1F1E33-float32/Tools

## Usage

### CatSystem2 Decryptor

```python
from tools_boost.catsystem2_decryptor import MT, Blowfish

mt = MT(12345)
random_number = mt.genrand()

key = b"your_key_here"
bf = Blowfish(key)
plaintext = bf.decrypt(ciphertext)
```

### HCA Decryptor

```python
from tools_boost.hca_decryptor import hca_decryptor_main

with open('encrypted.hca', 'rb') as f:
    encrypted_data = f.read()

decrypted_data = hca_decryptor_main(encrypted_data, mainkey=0x12345678, subkey=None)

with open('decrypted.hca', 'wb') as f:
    f.write(decrypted_data)
```

### XPCM2PCM

```python
from tools_boost.xpcm2pcm import xpcm2pcm_main
import struct

with open('audio.xpcm', 'rb') as f:
    xpcm_data = f.read()

pcm_data, meta = xpcm2pcm_main(xpcm_data)

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

### OpusNX2Opus

```python
from tools_boost.opusnx2opus import opusnx2opus_main

with open('input.opusnx', 'rb') as f:
    nx_data = f.read()

ogg_opus_data = opusnx2opus_main(nx_data)

with open('output.ogg', 'wb') as f:
    f.write(ogg_opus_data)
```

## References
- https://github.com/vgmstream/vgmstream
- https://github.com/nanami5270/GARbro-Mod
