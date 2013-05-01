#!/bin/bash


current_dir=`dirname $0`
source $current_dir/crawl-envs
usage(){
	echo "********************************************
	usage:
	$0 --prepareToCrawl <site-name> #prepare the site for crawl
	$0 --crawl <site-name> <depth> <topN(optional)>#start crawl
	$0 --help #print this message
	**************************************************"
	exit 255
}

prepare_to_crawl(){
	if [[ -d $CRAWL_CONF_DIR/$site_name ]];then
		echo "Crawl conf for site: $site_name already exists: $CRAWL_CONF_DIR/$site_name"
		exit 255
	fi
	mkdir -pv $CRAWL_CONF_DIR/$site_name
	cp -r $MY_NUTCH_HOME/conf/* $CRAWL_CONF_DIR/$site_name/
	echo "Add seed urls and configure regex-urlfilter.txt at: $CRAWL_CONF_DIR/$site_name"
}

crawl(){
	crawl_ts=`date '+%Y-%m-%d-%H-%M-%s'`
	crawl_dir=$CRAWL_DATA_DIR/$site_name-$crawl_ts
	mkdir -p $crawl_dir
	symlinks=("plugins" "logs" "bin" "lib")
	for file in ${symlinks[@]};do
		ln -sv $MY_NUTCH_HOME/$file $CRAWL_DATA_DIR/$site_name-$crawl_ts/
	done
	mkdir $crawl_dir/conf
	cp -r $CRAWL_CONF_DIR/$site_name/*  $crawl_dir/conf/

	echo "Crawl log at: $crawl_dir/crawl.log"
	if [[ -z $topN ]];then
		cd $crawl_dir && ./bin/nutch crawl conf/urls -dir crawl -depth $crawl_depth >$crawl_dir/crawl.log
	else
		cd $crawl_dir && ./bin/nutch crawl conf/urls -dir crawl -depth $crawl_depth -topN $topN >$crawl_dir/crawl.log
	fi
	echo "Crawl data is available at: $crawl_dir"
}

if [[ $1 = --prepareToCrawl ]];then
	if [[ $# != 2 ]];then usage ;fi
	echo "Preparing for crawl......."
	site_name=$2
	prepare_to_crawl
elif [[ $1 = --crawl ]];then
	if [[ $# != 3 ]] && [[ $# != 4 ]];then usage ;fi
	echo "Crawl will start soon........"
	site_name=$2
	crawl_depth=$3
	topN=$4
	crawl
else
	usage
fi


