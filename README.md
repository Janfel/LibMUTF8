# LibMUTF8

This library implements the encoding and decoding of Java’s Modified UTF-8
Encoding, which I’ll just call MUTF-8 from now on. The spec for MUTF-8 can be
found at <https://docs.oracle.com/javase/8/docs/api/java/io/DataInput.html>.
There are two major differences between MUTF-8 and regular UTF-8:

1. The null character (U+0000) is encoded as a two byte sequence (`C0 80`).
2. Instead of using four byte sequences, MUTF-8 encodes codepoints outside of
   the BMP using UTF-8 encoded UTF-16 surrogate pairs.

As such, MUTF-8 is really just UTF-16 in disguise, which makes the
conversion rather more painless than expected. As a tradeoff, this
implementation only validates the input on the code unit level, but not on
the codepoint level. Validating against unpaired surrogate codepoints is
left to the UTF-16 implementation, we just pass along what is given to us.

This library uses a MUTF-8 decoder based on work by Björn Höhrmann, see
<https://bjoern.hoehrmann.de/utf-8/decoder/dfa/>.
