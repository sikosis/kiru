NAME := Kiru
TYPE := APP
SRCS := src/KiruApp.cpp src/KiruWindow.cpp src/AboutWindow.cpp src/VideoPlayer.cpp src/VideoView.cpp src/VideoCutter.cpp
OBJS := $(SRCS:.cpp=.o)
RDEF := resources/Kiru.rdef
RSRC := $(RDEF:.rdef=.rsrc)
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -Wno-multichar -Isrc
LIBS := -lbe -lmedia -ltracker

.PHONY: all clean install check

all: $(NAME)

$(NAME): $(OBJS) $(RSRC)
	$(CXX) $(OBJS) -o $@ $(LIBS)
	xres -o $@ $(RSRC)
	mimeset -f $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.rsrc: %.rdef
	rc -o $@ $<

install: $(NAME)
	mkdir -p /boot/home/config/non-packaged/apps
	cp $(NAME) /boot/home/config/non-packaged/apps/$(NAME)

check:
	sh tests/smoke.sh

clean:
	rm -f $(OBJS) $(RSRC) $(NAME)
