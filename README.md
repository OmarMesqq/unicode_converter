## Tool for converting UTF-32 files to UTF-8 and vice versa

### Usage
Firstly, build it:
```
make release
```

Then, supply a text file that's encoded in any of the below:
- UTF-8
- UTF-32 Little Endian
- UTF-32 Big Endian

as a CLI argument, with:
```
./utfConverter <path/to/text_file>
```

Afterwards, the converted file will be available at a newly created 
folder `generated` at the project's root. Such file will be prepended 
with the tag `CONV_`, followed by the original filename.

Based on a [college assignment](https://web.archive.org/web/20260104124427/www.inf.puc-rio.br/~inf1018/2024.2/trabs/t1/trab1-20242.html). Feel free to learn more!
