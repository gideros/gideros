/*
Copyright (c) 2011 Juan Mellado

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
References:
- "LZMA SDK" by Igor Pavlov
  http://www.7-zip.org/sdk.html
- "The .lzma File Format" from xz documentation
  https://github.com/joachimmetz/xz/blob/master/doc/lzma-file-format.txt
*/

var LZMA = LZMA || {};

(function(LZMA) {

"use strict";

LZMA.OutWindow = function(){
  this._windowSize = 0;
};

LZMA.OutWindow.prototype.create = function(windowSize){
  if ( (!this._buffer) || (this._windowSize !== windowSize) ){
    // using a typed array here gives a big boost on Firefox
    // not much change in chrome (but more memory efficient)
    this._buffer = new Uint8Array(windowSize);
  }
  this._windowSize = windowSize;
  this._pos = 0;
  this._streamPos = 0;
};

LZMA.OutWindow.prototype.flush = function(){
  var size = this._pos - this._streamPos;
  if (size !== 0){
    if (this._stream.writeBytes){
      this._stream.writeBytes(this._buffer, size);
    } else {
      for (var i = 0; i < size; i ++){
        this._stream.writeByte(this._buffer[i]);
      }
    }
    if (this._pos >= this._windowSize){
      this._pos = 0;
    }
    this._streamPos = this._pos;
  }
};

LZMA.OutWindow.prototype.releaseStream = function(){
  this.flush();
  this._stream = null;
};

LZMA.OutWindow.prototype.setStream = function(stream){
  this.releaseStream();
  this._stream = stream;
};

LZMA.OutWindow.prototype.init = function(solid){
  if (!solid){
    this._streamPos = 0;
    this._pos = 0;
  }
};

LZMA.OutWindow.prototype.copyBlock = function(distance, len){
  var pos = this._pos - distance - 1;
  if (pos < 0){
    pos += this._windowSize;
  }
  while(len --){
    if (pos >= this._windowSize){
      pos = 0;
    }
    this._buffer[this._pos ++] = this._buffer[pos ++];
    if (this._pos >= this._windowSize){
      this.flush();
    }
  }
};

LZMA.OutWindow.prototype.putByte = function(b){
  this._buffer[this._pos ++] = b;
  if (this._pos >= this._windowSize){
    this.flush();
  }
};

LZMA.OutWindow.prototype.getByte = function(distance){
  var pos = this._pos - distance - 1;
  if (pos < 0){
    pos += this._windowSize;
  }
  return this._buffer[pos];
};

LZMA.RangeDecoder = function(){
};

LZMA.RangeDecoder.prototype.setStream = function(stream){
  this._stream = stream;
};

LZMA.RangeDecoder.prototype.releaseStream = function(){
  this._stream = null;
};

LZMA.RangeDecoder.prototype.init = function(){
  var i = 5;

  this._code = 0;
  this._range = -1;

  while(i --){
    this._code = (this._code << 8) | this._stream.readByte();
  }
};

LZMA.RangeDecoder.prototype.decodeDirectBits = function(numTotalBits){
  var result = 0, i = numTotalBits, t;

  while(i --){
    this._range >>>= 1;
    t = (this._code - this._range) >>> 31;
    this._code -= this._range & (t - 1);
    result = (result << 1) | (1 - t);

    if ( (this._range & 0xff000000) === 0){
      this._code = (this._code << 8) | this._stream.readByte();
      this._range <<= 8;
    }
  }

  return result;
};

LZMA.RangeDecoder.prototype.decodeBit = function(probs, index){
  var prob = probs[index],
      newBound = (this._range >>> 11) * prob;

  if ( (this._code ^ 0x80000000) < (newBound ^ 0x80000000) ){
    this._range = newBound;
    probs[index] += (2048 - prob) >>> 5;
    if ( (this._range & 0xff000000) === 0){
      this._code = (this._code << 8) | this._stream.readByte();
      this._range <<= 8;
    }
    return 0;
  }

  this._range -= newBound;
  this._code -= newBound;
  probs[index] -= prob >>> 5;
  if ( (this._range & 0xff000000) === 0){
    this._code = (this._code << 8) | this._stream.readByte();
    this._range <<= 8;
  }
  return 1;
};

LZMA.initBitModels = function(probs, len){
  while(len --){
    probs[len] = 1024;
  }
};

LZMA.BitTreeDecoder = function(numBitLevels){
  this._models = [];
  this._numBitLevels = numBitLevels;
};

LZMA.BitTreeDecoder.prototype.init = function(){
  LZMA.initBitModels(this._models, 1 << this._numBitLevels);
};

LZMA.BitTreeDecoder.prototype.decode = function(rangeDecoder){
  var m = 1, i = this._numBitLevels;

  while(i --){
    m = (m << 1) | rangeDecoder.decodeBit(this._models, m);
  }
  return m - (1 << this._numBitLevels);
};

LZMA.BitTreeDecoder.prototype.reverseDecode = function(rangeDecoder){
  var m = 1, symbol = 0, i = 0, bit;

  for (; i < this._numBitLevels; ++ i){
    bit = rangeDecoder.decodeBit(this._models, m);
    m = (m << 1) | bit;
    symbol |= bit << i;
  }
  return symbol;
};

LZMA.reverseDecode2 = function(models, startIndex, rangeDecoder, numBitLevels){
  var m = 1, symbol = 0, i = 0, bit;

  for (; i < numBitLevels; ++ i){
    bit = rangeDecoder.decodeBit(models, startIndex + m);
    m = (m << 1) | bit;
    symbol |= bit << i;
  }
  return symbol;
};

LZMA.LenDecoder = function(){
  this._choice = [];
  this._lowCoder = [];
  this._midCoder = [];
  this._highCoder = new LZMA.BitTreeDecoder(8);
  this._numPosStates = 0;
};

LZMA.LenDecoder.prototype.create = function(numPosStates){
  for (; this._numPosStates < numPosStates; ++ this._numPosStates){
    this._lowCoder[this._numPosStates] = new LZMA.BitTreeDecoder(3);
    this._midCoder[this._numPosStates] = new LZMA.BitTreeDecoder(3);
  }
};

LZMA.LenDecoder.prototype.init = function(){
  var i = this._numPosStates;
  LZMA.initBitModels(this._choice, 2);
  while(i --){
    this._lowCoder[i].init();
    this._midCoder[i].init();
  }
  this._highCoder.init();
};

LZMA.LenDecoder.prototype.decode = function(rangeDecoder, posState){
  if (rangeDecoder.decodeBit(this._choice, 0) === 0){
    return this._lowCoder[posState].decode(rangeDecoder);
  }
  if (rangeDecoder.decodeBit(this._choice, 1) === 0){
    return 8 + this._midCoder[posState].decode(rangeDecoder);
  }
  return 16 + this._highCoder.decode(rangeDecoder);
};

LZMA.Decoder2 = function(){
  this._decoders = [];
};

LZMA.Decoder2.prototype.init = function(){
  LZMA.initBitModels(this._decoders, 0x300);
};

LZMA.Decoder2.prototype.decodeNormal = function(rangeDecoder){
  var symbol = 1;

  do{
    symbol = (symbol << 1) | rangeDecoder.decodeBit(this._decoders, symbol);
  }while(symbol < 0x100);

  return symbol & 0xff;
};

LZMA.Decoder2.prototype.decodeWithMatchByte = function(rangeDecoder, matchByte){
  var symbol = 1, matchBit, bit;

  do{
    matchBit = (matchByte >> 7) & 1;
    matchByte <<= 1;
    bit = rangeDecoder.decodeBit(this._decoders, ( (1 + matchBit) << 8) + symbol);
    symbol = (symbol << 1) | bit;
    if (matchBit !== bit){
      while(symbol < 0x100){
        symbol = (symbol << 1) | rangeDecoder.decodeBit(this._decoders, symbol);
      }
      break;
    }
  }while(symbol < 0x100);

  return symbol & 0xff;
};

LZMA.LiteralDecoder = function(){
};

LZMA.LiteralDecoder.prototype.create = function(numPosBits, numPrevBits){
  var i;

  if (this._coders
    && (this._numPrevBits === numPrevBits)
    && (this._numPosBits === numPosBits) ){
    return;
  }
  this._numPosBits = numPosBits;
  this._posMask = (1 << numPosBits) - 1;
  this._numPrevBits = numPrevBits;

  this._coders = [];

  i = 1 << (this._numPrevBits + this._numPosBits);
  while(i --){
    this._coders[i] = new LZMA.Decoder2();
  }
};

LZMA.LiteralDecoder.prototype.init = function(){
  var i = 1 << (this._numPrevBits + this._numPosBits);
  while(i --){
    this._coders[i].init();
  }
};

LZMA.LiteralDecoder.prototype.getDecoder = function(pos, prevByte){
  return this._coders[( (pos & this._posMask) << this._numPrevBits)
    + ( (prevByte & 0xff) >>> (8 - this._numPrevBits) )];
};

LZMA.Decoder = function(){
  this._outWindow = new LZMA.OutWindow();
  this._rangeDecoder = new LZMA.RangeDecoder();
  this._isMatchDecoders = [];
  this._isRepDecoders = [];
  this._isRepG0Decoders = [];
  this._isRepG1Decoders = [];
  this._isRepG2Decoders = [];
  this._isRep0LongDecoders = [];
  this._posSlotDecoder = [];
  this._posDecoders = [];
  this._posAlignDecoder = new LZMA.BitTreeDecoder(4);
  this._lenDecoder = new LZMA.LenDecoder();
  this._repLenDecoder = new LZMA.LenDecoder();
  this._literalDecoder = new LZMA.LiteralDecoder();
  this._dictionarySize = -1;
  this._dictionarySizeCheck = -1;

  this._posSlotDecoder[0] = new LZMA.BitTreeDecoder(6);
  this._posSlotDecoder[1] = new LZMA.BitTreeDecoder(6);
  this._posSlotDecoder[2] = new LZMA.BitTreeDecoder(6);
  this._posSlotDecoder[3] = new LZMA.BitTreeDecoder(6);
};

LZMA.Decoder.prototype.setDictionarySize = function(dictionarySize){
  if (dictionarySize < 0){
    return false;
  }
  if (this._dictionarySize !== dictionarySize){
    this._dictionarySize = dictionarySize;
    this._dictionarySizeCheck = Math.max(this._dictionarySize, 1);
    this._outWindow.create( Math.max(this._dictionarySizeCheck, 4096) );
  }
  return true;
};

LZMA.Decoder.prototype.setLcLpPb = function(lc, lp, pb){
  var numPosStates = 1 << pb;

  if (lc > 8 || lp > 4 || pb > 4){
    return false;
  }

  this._literalDecoder.create(lp, lc);

  this._lenDecoder.create(numPosStates);
  this._repLenDecoder.create(numPosStates);
  this._posStateMask = numPosStates - 1;

  return true;
};

LZMA.Decoder.prototype.setProperties = function(props){
  if ( !this.setLcLpPb(props.lc, props.lp, props.pb) ){
    throw Error("Incorrect stream properties");
  }
  if ( !this.setDictionarySize(props.dictionarySize) ){
    throw Error("Invalid dictionary size");
  }
};

LZMA.Decoder.prototype.decodeHeader = function(inStream){

  var properties, lc, lp, pb,
      uncompressedSize,
      dictionarySize;

  if (inStream.size < 13){
    return false;
  }

  // +------------+----+----+----+----+--+--+--+--+--+--+--+--+
  // | Properties |  Dictionary Size  |   Uncompressed Size   |
  // +------------+----+----+----+----+--+--+--+--+--+--+--+--+

  properties = inStream.readByte();
  lc = properties % 9;
  properties = ~~(properties / 9);
  lp = properties % 5;
  pb = ~~(properties / 5);

  dictionarySize = inStream.readByte();
  dictionarySize |= inStream.readByte() << 8;
  dictionarySize |= inStream.readByte() << 16;
  dictionarySize += inStream.readByte() * 16777216;

  uncompressedSize = inStream.readByte();
  uncompressedSize |= inStream.readByte() << 8;
  uncompressedSize |= inStream.readByte() << 16;
  uncompressedSize += inStream.readByte() * 16777216;

  inStream.readByte();
  inStream.readByte();
  inStream.readByte();
  inStream.readByte();

  return {
    // The number of high bits of the previous
    // byte to use as a context for literal encoding.
    lc: lc,
    // The number of low bits of the dictionary
    // position to include in literal_pos_state.
    lp: lp,
    // The number of low bits of the dictionary
    // position to include in pos_state.
    pb: pb,
    // Dictionary Size is stored as an unsigned 32-bit
    // little endian integer. Any 32-bit value is possible,
    // but for maximum portability, only sizes of 2^n and
    // 2^n + 2^(n-1) should be used.
    dictionarySize: dictionarySize,
    // Uncompressed Size is stored as unsigned 64-bit little
    // endian integer. A special value of 0xFFFF_FFFF_FFFF_FFFF
    // indicates that Uncompressed Size is unknown.
    uncompressedSize: uncompressedSize
  };
};

LZMA.Decoder.prototype.init = function(){
  var i = 4;

  this._outWindow.init(false);

  LZMA.initBitModels(this._isMatchDecoders, 192);
  LZMA.initBitModels(this._isRep0LongDecoders, 192);
  LZMA.initBitModels(this._isRepDecoders, 12);
  LZMA.initBitModels(this._isRepG0Decoders, 12);
  LZMA.initBitModels(this._isRepG1Decoders, 12);
  LZMA.initBitModels(this._isRepG2Decoders, 12);
  LZMA.initBitModels(this._posDecoders, 114);

  this._literalDecoder.init();

  while(i --){
    this._posSlotDecoder[i].init();
  }

  this._lenDecoder.init();
  this._repLenDecoder.init();
  this._posAlignDecoder.init();
  this._rangeDecoder.init();
};

LZMA.Decoder.prototype.decodeBody = function(inStream, outStream, maxSize){
  var state = 0, rep0 = 0, rep1 = 0, rep2 = 0, rep3 = 0, nowPos64 = 0, prevByte = 0,
      posState, decoder2, len, distance, posSlot, numDirectBits;

  this._rangeDecoder.setStream(inStream);
  this._outWindow.setStream(outStream);

  this.init();

  while(maxSize < 0 || nowPos64 < maxSize){
    posState = nowPos64 & this._posStateMask;

    if (this._rangeDecoder.decodeBit(this._isMatchDecoders, (state << 4) + posState) === 0){
      decoder2 = this._literalDecoder.getDecoder(nowPos64 ++, prevByte);

      if (state >= 7){
        prevByte = decoder2.decodeWithMatchByte(this._rangeDecoder, this._outWindow.getByte(rep0) );
      }else{
        prevByte = decoder2.decodeNormal(this._rangeDecoder);
      }
      this._outWindow.putByte(prevByte);

      state = state < 4? 0: state - (state < 10? 3: 6);

    }else{

      if (this._rangeDecoder.decodeBit(this._isRepDecoders, state) === 1){
        len = 0;
        if (this._rangeDecoder.decodeBit(this._isRepG0Decoders, state) === 0){
          if (this._rangeDecoder.decodeBit(this._isRep0LongDecoders, (state << 4) + posState) === 0){
            state = state < 7? 9: 11;
            len = 1;
          }
        }else{
          if (this._rangeDecoder.decodeBit(this._isRepG1Decoders, state) === 0){
            distance = rep1;
          }else{
            if (this._rangeDecoder.decodeBit(this._isRepG2Decoders, state) === 0){
              distance = rep2;
            }else{
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        if (len === 0){
          len = 2 + this._repLenDecoder.decode(this._rangeDecoder, posState);
          state = state < 7? 8: 11;
        }
      }else{
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;

        len = 2 + this._lenDecoder.decode(this._rangeDecoder, posState);
        state = state < 7? 7: 10;

        posSlot = this._posSlotDecoder[len <= 5? len - 2: 3].decode(this._rangeDecoder);
        if (posSlot >= 4){

          numDirectBits = (posSlot >> 1) - 1;
          rep0 = (2 | (posSlot & 1) ) << numDirectBits;

          if (posSlot < 14){
            rep0 += LZMA.reverseDecode2(this._posDecoders,
                rep0 - posSlot - 1, this._rangeDecoder, numDirectBits);
          }else{
            rep0 += this._rangeDecoder.decodeDirectBits(numDirectBits - 4) << 4;
            rep0 += this._posAlignDecoder.reverseDecode(this._rangeDecoder);
            if (rep0 < 0){
              if (rep0 === -1){
                break;
              }
              return false;
            }
          }
        }else{
          rep0 = posSlot;
        }
      }

      if (rep0 >= nowPos64 || rep0 >= this._dictionarySizeCheck){
        return false;
      }

      this._outWindow.copyBlock(rep0, len);
      nowPos64 += len;
      prevByte = this._outWindow.getByte(0);
    }
  }

  this._outWindow.flush();
  this._outWindow.releaseStream();
  this._rangeDecoder.releaseStream();

  return true;
};

LZMA.Decoder.prototype.setDecoderProperties = function(properties){
  var value, lc, lp, pb, dictionarySize;

  if (properties.size < 5){
    return false;
  }

  value = properties.readByte();
  lc = value % 9;
  value = ~~(value / 9);
  lp = value % 5;
  pb = ~~(value / 5);

  if ( !this.setLcLpPb(lc, lp, pb) ){
    return false;
  }

  dictionarySize = properties.readByte();
  dictionarySize |= properties.readByte() << 8;
  dictionarySize |= properties.readByte() << 16;
  dictionarySize += properties.readByte() * 16777216;

  return this.setDictionarySize(dictionarySize);
};

LZMA.decompress = function(properties, inStream, outStream, outSize){
  var decoder = new LZMA.Decoder();

  if ( !decoder.setDecoderProperties(properties) ){
    throw Error("Incorrect lzma stream properties");
  }

  if ( !decoder.decodeBody(inStream, outStream, outSize) ){
    throw Error("Error in lzma data stream");
  }

  return outStream;
};

LZMA.decompressFile = function(inStream, outStream){
  // upgrade ArrayBuffer to input stream
  if (inStream instanceof ArrayBuffer) {
    inStream = new LZMA.iStream(inStream);
  }
  // optionaly create a new output stream
  if (!outStream && LZMA.oStream) {
    outStream = new LZMA.oStream();
  }
  // create main decoder instance
  var decoder = new LZMA.Decoder();
  // read all the header properties
  var header = decoder.decodeHeader(inStream);
  // get maximum output size (very big!?)
  var maxSize = header.uncompressedSize;
  // setup/init decoder states
  decoder.setProperties(header);
  // invoke the main decoder function
  if ( !decoder.decodeBody(inStream, outStream, maxSize) ){
    // only generic error given here
    throw Error("Error in lzma data stream");
  }
  // return result
  return outStream;
};

LZMA.decode = LZMA.decompressFile;

})(LZMA);

/*
Copyright (c) 2017 Marcel Greter (http://github.com/mgreter)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

var LZMA = LZMA || {};

(function (LZMA) {

	// very simple in memory input stream class
	LZMA.iStream = function(buffer)
	{
		// create byte array view of buffer
		this.array = new Uint8Array(buffer);
		// convenience status member
		this.size = buffer.byteLength;
		// position pointer
		this.offset = 0;
	}

	// simply return the next byte from memory
	LZMA.iStream.prototype.readByte = function()
	{
		// advance pointer and return byte
		return this.array[this.offset++];
	}

	// output stream constructor
	LZMA.oStream = function(buffers)
	{
		// aggregated size
		this.size = 0;
		// initialize empty
		this.buffers = [];
		buffers = buffers || [];
		// make sure size matches data
		for (var i = 0, L = buffers.length; i < L; i++) {
			// unwrap nested output streams
			if (buffers[i] instanceof LZMA.oStream) {
				var oBuffers = buffers[i].buffers;
				for (var n = 0; n < oBuffers.length; n++) {
					this.buffers.push(buffers[i].buffers[n]);
					this.size += buffers[i].buffers[n].length;
				}
			} else {
				// simply append the one buffer
				this.buffers.push(buffers[i]);
				this.size += buffers[i].length;
			}
		}
	}

	// we expect a Uint8Array buffer and the size to read from
	// creates a copy of the buffer as needed so you can re-use it
	// tests with js-lzma have shown that this is at most for 16MB
	LZMA.oStream.prototype.writeBytes = function writeBytes(buffer, size)
	{
		// can we just take the full buffer?
		// or just some part of the buffer?
		if (size <= buffer.byteLength) {
			// we need to make a copy, as the original
			// buffer will be re-used. No way around!
			this.buffers.push(buffer.slice(0, size));
		}
		// assertion for out of boundary access
		else { throw Error("Buffer too small?"); }
		// increase counter
		this.size += size;
	}

	// return a continous Uint8Array with the full content
	// the typed array is guaranteed to have to correct length
	// also meaning that there is no space remaining to add more
	// you may should expect malloc errors if size gets a few 10MB
	// calling this repeatedly always returns the same array instance
	// NOTE: An alternative approach would be to use a Blob. A Blob
	// can be created out of an array of array chunks (our buffers).
	// Via a FileReader we can then convert it back to a continous
	// Uint8Array. But this would make this method async in nature!
	LZMA.oStream.prototype.toUint8Array = function toUint8Array()
	{
		// local variable access
		var size = this.size,
			buffers = this.buffers;

		// the simple case with only one buffer
		if (buffers.length == 1) {
			// make a copy if needed!
			return buffers[0];
		}
		// otherwise we need to concat them all now
		try {
			// allocate the continous memory chunk
			var continous = new Uint8Array(size);
			// process each buffer in the output queue
			for (var i = 0, offset = 0; i < buffers.length; i++) {
				continous.set(buffers[i], offset);
				offset += buffers[i].length;
			}
			// release memory chunks
			buffers[0] = continous;
			// only one chunk left
			buffers.length = 1;
			// return typed array
			return continous;
			// Asynchronous alternative:
			// var blob = new Blob(outStream.buffers);
			// var reader = new FileReader();
			// reader.onload = function() { ... };
			// reader.readAsArrayBuffer(blob);
		}
		// probably allocation error
		catch (err) {
			// this error is somewhat expected so you should take care of it
			console.error("Error allocating Uint8Array of size: ", size);
			console.error("Message given was: ", err.toString());
		}
		// malloc error
		return null;
	}

	// invoke fn on every Uint8Array in the stream
	// using this interface can avoid the need to
	// create a full continous buffer of the result
	LZMA.oStream.prototype.forEach = function forEach(fn)
	{
		for (var i = 0; i < this.buffers.length; i++) {
			fn.call(this, this.buffers[i]);
		}
	}

	// returns a typed array of codepoints; depending if
	// UTF8 decoder is loaded, we treat the byte sequence
	// either as an UTF8 sequence or fixed one byte encoding
	// the result can then be converted back to a JS string
	LZMA.oStream.prototype.toCodePoints = function toCodePoints()
	{
		// treat as one byte encoding (i.e. US-ASCII)
		if (!LZMA.UTF8) { this.toUint8Array(); }
		// we could probably make this work with our chunked
		// buffers directly, but unsure how much we could gain
		return LZMA.UTF8.decode(this.toUint8Array());
	}

	// convert the buffer to a javascript string object
	LZMA.oStream.prototype.toString = function toString()
	{
		var buffers = this.buffers, string = '';
		// optionally get the UTF8 codepoints
		// possibly avoid creating a continous buffer
		if (LZMA.UTF8) buffers = [ this.toCodePoints() ];
		for (var n = 0, nL = buffers.length; n < nL; n++) {
			for (var i = 0, iL = buffers[n].length; i < iL; i++) {
				string += String.fromCharCode(buffers[n][i]);
			}
		}
		return string;
	}

})(LZMA);
