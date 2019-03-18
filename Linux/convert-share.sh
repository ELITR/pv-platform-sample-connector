#!/bin/bash


origdir=$1
[ -z $origdir ] && origdir=segments-test

wavdir=$2
[ -z $wavdir ] && wavdir=$origdir""_wav

asrdir=$3
[ -z $asrdir ] && asdir=$origdir""_asr

olderthan() {
	file=$1
	fmtime=`stat -c %Y $file`
	now=`date +%s`
	if [[ $(($now-$fmtime)) -gt 10 ]]; then
		return 0
	else
		return 1
	fi
}


asr() {
	wav=$1
	txt=$2
	echo /home/d/Plocha/elitr/pythonrecordingclient/p2/bin/recording_client_file.py \
		-S i13srv30.ira.uka.de -p 4443 \
		-fi en-EU-lecture_KIT -fo us -po \
		-f $wav -pf $txt | bash
}

for f in $origdir/seg*/*.pcm; do
	[[ ! -f $f ]] && continue
	echo $f
	if olderthan $f; then
		wavf_pcm=${f/$origdir/$wavdir}
		txtf_pcm=${f/$origdir/$asrdir}
		wavf=${wavf_pcm/pcm/wav}
		txtf=${txtf_pcm/pcm/txt}
		mkdir -p `dirname $wavf`
		mkdir -p `dirname $txtf`

		if [ ! -f $wavf ]; then
			ffmpeg -f s16le -ar 16k -ac 1 -i $f $wavf
			echo creating $wavf
			asr $wavf $txtf
		fi
	else
		echo not
	fi
done

echo $orig
