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

#include "StreamSource.hpp"
#include "AudioFormatAdaptor.hpp"

StreamSource::StreamSource(AudioFormat format, std::shared_ptr<IStream> pStream): ISource(), mFormat(format), mpStream(pStream)
{

}

StreamSource::~StreamSource()
{
  close();
}

void StreamSource::close(void)
{
  if( mpStream ){
    mpStream->close();
    mpStream.reset();
  }
}

void StreamSource::parse(ByteBuffer& inStreamBuf, IAudioBuffer& dstAudioBuf)
{
  // TODO: serialize the inStreamBuf as the expected format and output to dstAudioBuf
  dstAudioBuf.setRawBuffer( inStreamBuf );
}


void StreamSource::readPrimitive(IAudioBuffer& buf)
{
  if( mpStream && !mpStream->isEndOfStream() ){
    ByteBuffer inStreamBuf( buf.getRawBuffer().size() );
    mpStream->read( inStreamBuf );
    parse( inStreamBuf, buf );

    // convert if necessary
    if( !mFormat.equal( buf.getAudioFormat() ) ){
      AudioBuffer* pBuf = dynamic_cast<AudioBuffer*>(&buf);
      if( pBuf ){
        AudioBuffer dstAudioBuffer( mFormat, pBuf->getNumberOfSamples() );
        AudioFormatAdaptor::convert( *pBuf, dstAudioBuffer );
        *pBuf = dstAudioBuffer;
      }
    }
  }
}

bool StreamSource::setAudioFormat(AudioFormat audioFormat)
{
  mFormat = audioFormat;
  // TODO: Change the parser's format.

  return true;
}

AudioFormat StreamSource::getAudioFormat(void)
{
  return mFormat;
}
