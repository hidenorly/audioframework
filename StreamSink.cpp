/* 
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "StreamSink.hpp"
#include "AudioFormatAdaptor.hpp"

StreamSink::StreamSink(AudioFormat format, IStream* pStream): mFormat(format), mpStream(pStream)
{

}

StreamSink::~StreamSink()
{
  close();
}

void StreamSink::close(void)
{
  if( mpStream ){
    mpStream->close();
    delete mpStream; mpStream = nullptr;
  }
}

void StreamSink::serialize(AudioBuffer& srcAudioBuf, ByteBuffer& outStreamBuf)
{
  ByteBuffer rawSrcBuffer = srcAudioBuf.getRawBuffer();
  // TODO: serialize to the expected format
  std::copy(rawSrcBuffer.begin(), rawSrcBuffer.end(), outStreamBuf.begin());
}


void StreamSink::write(AudioBuffer& buf)
{
  if( mpStream ){
    // convert if necessary
    if( !mFormat.equal( buf.getAudioFormat() ) ){
      AudioBuffer dstAudioBuffer( mFormat, buf.getSamples() );
      AudioFormatAdaptor::convert( buf, dstAudioBuffer );
      buf = dstAudioBuffer;
    }
 
    ByteBuffer outStreamBuf( buf.getRawBuffer().size() );
    serialize( buf, outStreamBuf );

    mpStream->write( outStreamBuf );
    mpStream = nullptr;
  }
}

bool StreamSink::setAudioFormat(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  // TODO: Change the serializer's format.

  return true;
}

AudioFormat StreamSink::getAudioFormat(void)
{
  return mFormat;
}
