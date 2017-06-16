#!/usr/bin/python
# -*- coding: utf-8 -*-
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('GdkX11', '3.0')
from gi.repository import Gtk, Gdk, GdkX11
import Xlib
from Xlib.display import Display
from Xlib import X

window = Gtk.Window(title="Secure Workstation Dock")
window.set_name("bar")
window.set_type_hint(Gdk.WindowTypeHint.DOCK)
window.set_decorated(False)

style_provider = Gtk.CssProvider()
style_provider.load_from_data(b"""
window#bar {
    background-color: red;
}""")
Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

screen = window.get_screen()
width = screen.width()
window.set_default_size(width, 100)
window.set_keep_above(True)

#area = Gtk.DrawingArea()
#area.set_size_request(width, 100);
#area.set_events(Gtk.gdk.POINTER_MOTION_MASK);

#def mouse_move_callback(area, event):
#    print(str(event))
#area.connect("motion_notify_event", mouse_move_callback);
#window.add(area)
#area.show()

label = Gtk.Label('<span size="38000" color="red">gmail.com</span>')
label.set_use_markup(True)
window.add(label)

window.connect("delete-event", Gtk.main_quit)
window.show_all()

# Reserve space on the top
display = Display()
topw = display.create_resource_object('window', window.get_toplevel().get_window().get_xid())
topw.change_property(display.intern_atom('_NET_WM_STRUT'), display.intern_atom('CARDINAL'), 32, [0, 0, 100, 0], X.PropModeReplace)
topw.change_property(display.intern_atom('_NET_WM_STRUCT_PARTIAL'), display.intern_atom('CARDINAL'), 32, [0, 0, 100, 0, 0 ,0 ,0, 0, width-1, 0, 0], X.PropModeReplace)

from threading import Thread
class KeyboardReaderThread(Thread):
    def run(self):
        from evdev import InputDevice, categorize, ecodes
        dev = InputDevice('/dev/input/by-id/usb-Apple__Inc_Apple_Keyboard-event-kbd')
        scancodes = {
            # Scancode: ASCIICode
            0: None, 1: u'ESC', 2: u'1', 3: u'2', 4: u'3', 5: u'4', 6: u'5', 7: u'6', 8: u'7', 9: u'8',
            10: u'9', 11: u'0', 12: u'-', 13: u'=', 14: u'BKSP', 15: u'TAB', 16: u'q', 17: u'w', 18: u'e', 19: u'r',
            20: u't', 21: u'y', 22: u'u', 23: u'i', 24: u'o', 25: u'p', 26: u'[', 27: u']', 28: u'CRLF', 29: u'LCTRL',
            30: u'a', 31: u's', 32: u'd', 33: u'f', 34: u'g', 35: u'h', 36: u'j', 37: u'k', 38: u'l', 39: u';',
            40: u'"', 41: u'`', 42: u'LSHFT', 43: u'\\', 44: u'z', 45: u'x', 46: u'c', 47: u'v', 48: u'b', 49: u'n',
            50: u'm', 51: u',', 52: u'.', 53: u'/', 54: u'RSHFT', 56: u'LALT', 57: u' ', 100: u'RALT'
        }
        capscodes = {
            0: None, 1: u'ESC', 2: u'!', 3: u'@', 4: u'#', 5: u'$', 6: u'%', 7: u'^', 8: u'&', 9: u'*',
            10: u'(', 11: u')', 12: u'_', 13: u'+', 14: u'BKSP', 15: u'TAB', 16: u'Q', 17: u'W', 18: u'E', 19: u'R',
            20: u'T', 21: u'Y', 22: u'U', 23: u'I', 24: u'O', 25: u'P', 26: u'{', 27: u'}', 28: u'CRLF', 29: u'LCTRL',
            30: u'A', 31: u'S', 32: u'D', 33: u'F', 34: u'G', 35: u'H', 36: u'J', 37: u'K', 38: u'L', 39: u':',
            40: u'\'', 41: u'~', 42: u'LSHFT', 43: u'|', 44: u'Z', 45: u'X', 46: u'C', 47: u'V', 48: u'B', 49: u'N',
            50: u'M', 51: u'<', 52: u'>', 53: u'?', 54: u'RSHFT', 56: u'LALT',  57: u' ', 100: u'RALT'
        }
        x = ''
        caps = False
        dev.grab()
        for event in dev.read_loop():
            if event.type == ecodes.EV_KEY:
                data = categorize(event)  # Save the event temporarily to introspect it
                if data.scancode == 42:
                    if data.keystate == 1:
                        caps = True
                    if data.keystate == 0:
                        caps = False
                if data.keystate == 1:  # Down events only
                    if caps:
                        key_lookup = u'{}'.format(capscodes.get(data.scancode)) or u'UNKNOWN:[{}]'.format(data.scancode)  # Lookup or return UNKNOWN:XX
                    else:
                        key_lookup = u'{}'.format(scancodes.get(data.scancode)) or u'UNKNOWN:[{}]'.format(data.scancode)  # Lookup or return UNKNOWN:XX
                    if (data.scancode != 42) and (data.scancode != 28):
                        x += key_lookup 
                        label.set_markup('<span size="38000" color="red">' + x + '</span>') 
                    if(data.scancode == 28):
                        print x          # Print it all out!
                        label.set_markup('<span size="38000" color="green">' + x + '</span>')
                        x = ''
kbdThread = KeyboardReaderThread()
kbdThread.start()

#Gtk.main()
from gi.repository import GLib
GLib.MainLoop().run()

