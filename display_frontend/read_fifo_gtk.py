#!/usr/bin/env python3
import sys
import numpy as np
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, Gdk, GLib

# set up paths to read from / write to FIFOs
FIFO_MEM__PATH = "/tmp/8080fifo"
FIFO_IN__PATH = "/tmp/8080fifo_in" # for sending data back

# fifo funcs
def get_memory_from_fifo():
    buffer = [0]
    with open(FIFO_MEM__PATH, "rb") as fifo_mem:
        data = fifo_mem.read()
        buffer = np.frombuffer(data, dtype=np.uint8)
    return buffer


class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.set_default_size(512,512);
        self.set_title("Frontend 8080");
        # create box for storing canvas
        self.box1 = Gtk.Box(orientation = Gtk.Orientation.VERTICAL)
        # self.set_child(self.box1)
        # create a surface to blit the memory contents to
        self.display = Gtk.Picture()
        self.on_draw(self.display)
        # self.box1.append(self.display)
        self.set_child(self.display)

    def on_draw(self, picture):
        # create buffer to cairo image surface
        buffer = get_memory_from_fifo()
        bytes = GLib.Bytes(buffer)
        print(bytes.get_size())
        tex = Gdk.MemoryTexture.new(256, 256, 20, bytes, 256)
        picture.set_paintable(tex)


class Frontend8080(Adw.Application):
    def __init__(self, *args, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application = app)
        self.win.present()


app = Frontend8080(application_id = "com.github.helpvisa.Frontend8080")
app.run(sys.argv)
