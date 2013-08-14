﻿/*

  Copyright (C) 2013 xubinbin 徐彬彬 (Beijing China)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  
  filename: main.c 
  version : v1.1.0
  time    : 2013/08/02 16:20 
  author  : xubinbin ( xubbwd@gmail.com )
  code URL: http://code.google.com/p/tsdemux/

*/

#include "fa_parseopt.h"
#include "fa_ts2es.h"

int main(int argc,char * argv[])
{

    int ret = 0;

	unsigned char buffer_ts_header[4];
	unsigned char pat[184];

	FILE * fp_sourcefile = NULL;
	FILE * fp_video_destfile = NULL;
	FILE * fp_audio_destfile = NULL;

    ret = fa_parseopt(argc, argv);
    if(ret) return -1;

	if ((fp_sourcefile = fopen(opt_inputfile, "rb")) == NULL) {
		printf("input file can not be opened;\n");
		return 0;
    }
	if(strlen(opt_video_outputfile) > 0) {
		if ((fp_video_destfile = fopen(opt_video_outputfile, "w+b")) == NULL) {
			printf("video output file can not be opened\n");
			return 0;
		}
	}
	if(strlen(opt_audio_outputfile) > 0) {
		if ((fp_audio_destfile = fopen(opt_audio_outputfile, "w+b")) == NULL) {
			printf("audio output file can not be opened\n");
			return 0;
		}
	}

//前面这里最好是写一个包数据的循环处理包出来
//循环读一个数据包，循环对包的PID进行判断

//ts的头的搜索

	ptsdemux = (tsdemux_struc *)malloc(sizeof(tsdemux_struc));
    TS_packet_header *packet_head = malloc(sizeof(TS_packet_header));
    TS_PAT * packet_pat =  malloc(sizeof(TS_PAT));
    TS_PMT * packet_pmt =  malloc(sizeof(TS_PMT));
	if (NULL == ptsdemux || packet_head == NULL || packet_pat == NULL || packet_pmt == NULL)
	{
		printf("malloc memory err!\n");
		return -1;
	}
	memset(ptsdemux, 0, sizeof(struct tsdemux_struc));
	memset(packet_head, 0, sizeof(struct _TS_packet_header));
	memset(packet_pat, 0, sizeof(struct TS_PAT));
	memset(packet_pmt, 0, sizeof(struct TS_PMT));
	
	ptsdemux->audio_pid=-1; /* disable all pids at start */
	ptsdemux->video_pid=-1;

//	int i = 0;
//    for(i = 0;i < 6;i++)
    while(1)
    {
        if(fread(buffer_ts_header,4,1,fp_sourcefile) != 1) {
        //	printf("read inputfile ts_head error !\n");
        	break;
        }
        adjust_TS_packet_header((TS_packet_header *)packet_head,buffer_ts_header);

        if(fread(pat,184,1,fp_sourcefile) != 1) {
        //	printf("read inputfile data error !\n");
        	break;
        }

//        printf("0x%x 0x%x 0x%x 0x%x  \n",buffer_ts_header[0],buffer_ts_header[1],buffer_ts_header[2],buffer_ts_header[3]);
//        printf("PID                                    \t### %d\n", packet_head->PID);
//        if(packet_head->PID == 1024)
//          printf("i = %d\n",i+1);
//        printf("payload_unit_start_indicator \t\t### %x\n", packet_head->payload_unit_start_indicator);

        //如果这个包的pid是0 可以通过pat表查找出pmt表的pid
        if(packet_head->PID  == 0)
        {
            adjust_PAT_table(packet_head,packet_pat,pat);
        }
        else if(packet_head->PID  == packet_pat->program_map_PID)
        {
            adjust_PMT_table(packet_head,packet_pmt,pat);
        }
        else if(packet_head->PID == ptsdemux->video_pid)
        {
        //    printf("V ");
        //    printf_TS_packet_header_info(packet_head);
        	if(strlen(opt_video_outputfile) > 0) {
        		adjust_video_table(packet_head,pat,fp_video_destfile);
        	}
        }
        else if(packet_head->PID == ptsdemux->audio_pid)
        {
    //        printf("A");
    //        printf_TS_packet_header_info(packet_head);
    //        packet_head->adaption_field_control = 1;
    //        packet_head->payload_unit_start_indicator = 0;
        	if(strlen(opt_audio_outputfile) > 0) {
        		adjust_video_table(packet_head,pat,fp_audio_destfile);
        	}
        }
        else
        {
            printf("error pid\n");
        }
    }

    fclose(fp_sourcefile);
    if(strlen(opt_video_outputfile) > 0) {
    	fclose(fp_video_destfile);
    }
    if(strlen(opt_audio_outputfile) > 0) {
        fclose(fp_audio_destfile);
    }

    printf("Done.\n");

    return 0;
}
