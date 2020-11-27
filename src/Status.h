#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

class Status{
private:
    //emulation status
	//mutable
	bool reset;

	char* post_title;
	bool post_ignore;
	int post_cpuspeed;
	int post_fps;
	int post_whichInterpreter;
	int post_flickerOffset;
	//immutable
	//!----end of emulation status
public:
    Status(const Status& other){
        reset = other.reset;
        post_ignore = other.post_ignore;
        post_cpuspeed = other.post_cpuspeed;
        post_fps = other.post_fps;
        post_whichInterpreter = other.post_whichInterpreter;
        post_flickerOffset = other.post_flickerOffset;
        set_post_title(other.post_title);
    }
    Status(){}

    //emulation status
    void set_reset(bool reset){ this->reset = reset; }
    void set_post_title(const char* str){
        if(this->post_title) free(this->post_title);
        this->post_title = (char*)malloc((strlen(str) + 1) * sizeof(char));
        strcpy(this->post_title, str);
    }
    void set_post_ignore(bool post_ignore){ this->post_ignore = post_ignore; }
    void set_post_cpuspeed(int post_cpuspeed){ this->post_cpuspeed = post_cpuspeed; }
    void set_post_fps(int post_fps){ this->post_fps = post_fps; }
    void set_post_whichInterpreter(int post_whichInterpreter){ this->post_whichInterpreter = post_whichInterpreter; }
    void set_post_flickerOffset(int post_flickerOffset){ this->post_flickerOffset = post_flickerOffset; }

    //getters
    bool get_reset(){ return reset; }
    const char* get_post_title(){ return post_title; }
    bool get_post_ignore(){ return post_ignore; }
    int get_post_cpuspeed(){ return post_cpuspeed; }
    int get_post_fps(){ return post_fps; }
    int get_post_whichInterpreter(){ return post_whichInterpreter; }
    int get_post_flickerOffset(){ return post_flickerOffset; }
};
