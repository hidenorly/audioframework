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

StreamSink::StreamSink(AudioFormat format, std::shared_ptr<IStream> pStream): ISink(), mFormat(format), mpStream(pStream)
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
    mpStream.reset();
  }
}

void StreamSink::serialize(IAudioBuffer& srcAudioBuf, ByteBuffer& outStreamBuf)
{
  ByteBuffer& rawSrcBuffer = srcAudioBuf.getRawBuffer();
  // TODO: serialize to the expected format
#if __USE_COPY_WITH_BACKINSERTER__
  std::copy(rawSrcBuffer.begin(), rawSrcBuffer.end(), outStreamBuf.begin());
#endif // __USE_COPY_WITH_BACKINSERTER__
#if __USE_INSERT__
  outStreamBuf.insert( outStreamBuf.begin(), rawSrcBuffer.begin(), rawSrcBuffer.end() );
#endif // __USE_INSERT__
}


void StreamSink::writePrimitive(IAudioBuffer& buf)
{
  if( mpStream ){
    // convert if necessary
    AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
    if( pBuf && !mFormat.equal( pBuf->getAudioFormat() ) ){
      AudioBuffer dstAudioBuffer( mFormat, pBuf->getNumberOfSamples() );
      AudioFormatAdaptor::convert( *pBuf, dstAudioBuffer );
      *pBuf = dstAudioBuffer;
    }
 
    ByteBuffer outStreamBuf( buf.getRawBufferSize() );
    serialize( buf, outStreamBuf );

    mpStream->write( outStreamBuf );
  }
}

void StreamSink::setAudioFormatPrimitive(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  // TODO: Change the serializer's format.
}

AudioFormat StreamSink::getAudioFormat(void)
{
  return mFormat;
}
