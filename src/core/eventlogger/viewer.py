#! /usr/bin/env python
#
# xrc generated with xrced (comes with wxPython)
#

import datetime
import logging
import optparse
import os
import sys
import threading
import time
import wx
import wx.aui
import wx.lib.intctrl
import wx.lib.mixins.listctrl
import wx.lib.newevent
import wx.xrc

# custom GUI events
ParseBeginEvent, EVT_PARSE_BEGIN = wx.lib.newevent.NewEvent()
ParseProgressEvent, EVT_PARSE_PROGRESS = wx.lib.newevent.NewEvent()
ParseFinishedEvent, EVT_PARSE_FINISHED = wx.lib.newevent.NewEvent()
ParseErrorEvent, EVT_PARSE_ERROR = wx.lib.newevent.NewEvent()
RebuildEventListEvent, EVT_REBUILD_EVENT_LIST = wx.lib.newevent.NewEvent()
EventDetails, EVT_EVENT_DETAILS = wx.lib.newevent.NewEvent()

def boolean_yes_no(value):
    return "Yes" if value else "No"

def read_bytes(buffer, count):
    # strip the number of bytes from the buffer
    bytes = buffer[:count]
    del buffer[:count]
    return bytes

def read_byte(buffer):
    return read_bytes(buffer, 1)

def read_string(buffer, len):
    return str(read_bytes(buffer, len))

def read_int(buffer):
    # little-endian
    bytes = read_bytes(buffer, 4)
    return ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff)

def read_long(buffer):
    # little-endian
    bytes = read_bytes(buffer, 8)
    return ((bytes[7] & 0xff) << 56) | ((bytes[6] & 0xff) << 48) | ((bytes[5] & 0xff) << 40) | ((bytes[4] & 0xff) << 32) | ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff)

class Borg(object):
    __shared_state = {}

    def __new__(cls, *p, **k):
        obj = object.__new__(cls, *p, **k)
        obj.__dict__ = cls.__shared_state
        return obj

class EventType(object):
    UNKNOWN = 0

    # NOTE: these must match the values in the XRC
    EVENT_NAMES = {
        UNKNOWN : "Unknown Event",
    }
    EVENT_NAMES_REV = dict((value, key) for key, value in EVENT_NAMES.iteritems())

    @staticmethod
    def create_event_type(type):
        return EventType()

    @staticmethod
    def type_to_str(type):
        if type in EventType.EVENT_NAMES:
            return EventType.EVENT_NAMES[type]
        return "Unknown"

    @staticmethod
    def type_from_str(type):
        if type in EventType.EVENT_NAMES_REV:
            return EventType.EVENT_NAMES_REV[type]
        return EventType.UNKNOWN

    def __init__(self, type=UNKNOWN):
        self.__type = type

    @property
    def type(self):
        return self.__type

    @property
    def version(self):
        return 1

    @property
    def type_name(self):
        return EventType.type_to_str(self.type)

    @property
    def unknown(self):
        return EventType.UNKNOWN == self.type

    @property
    def preview(self):
        return ""

    def deserialize(self, buffer):
        return True

    def __str__(self):
        return "Unknown Event Type!"

class Event(object):
    class Header(object):
        MAGIC = "EVT"
        VERSION = 1

        def __init__(self, app):
            self.__app = app

        def deserialize(self, buffer):
            magic = read_bytes(buffer, 8)
            if self.MAGIC != magic:
                wx.PostEvent(self.__app, ParseErrorEvent(message="Invalid MAGIC!"))
                return False

            version = read_int(buffer)
            if self.VERSION != version:
                wx.PostEvent(self.__app, ParseErrorEvent(message="Invalid VERSION: %d" % (version)))
                return False

            return True

    def __init__(self, app):
        self.__logger = logging.getLogger("viewer.Event")

        self.__app = app
        self.__id = 0
        self.__timestamp = 0
        self.__type = EventType()

    @property
    def valid(self):
        return self.__id > 0

    @property
    def id(self):
        return self.__id

    @property
    def timestamp(self):
        return self.__timestamp

    @property
    def timestamp_str(self):
        timestamp = datetime.datetime.fromtimestamp(self.timestamp)
        return timestamp.strftime("%a %b %d %H:%M:%S %Z %Y")

    @property
    def type(self):
        return self.__type

    def deserialize(self, buffer):
        header = Event.Header(self.__app)
        if not header.deserialize(buffer):
            return False
        self.__id = read_long(buffer)

        # java timestamps are millisecond
        # so we gotta cut some precision
        self.__timestamp = (read_long(buffer) / 1000)

        # TODO: we know the size of the content here
        # so if we don't recognize a type, we should
        # just go ahead and skip that event somehow

        type = read_int(buffer)
        self.__type = EventType.create_event_type(type)
        if self.__type.type != type:
            wx.PostEvent(self.__app, ParseErrorEvent(message="Event type mismatch!"))
            #self.__logger.warning("Event type mismatch!")
            return False

        version = read_int(buffer)
        if self.__type.version != version:
            wx.PostEvent(self.__app, ParseErrorEvent(message="Event version mismatch!"))
            return False

        return self.__type.deserialize(buffer)

    def __getitem__(self, key):
        if 0 == key:
            return self.id
        elif 1 == key:
            return self.timestamp
        elif 2 == key:
            return self.type.type_name
        return self.type[key]

    def __str__(self):
        return "Event: timestamp=%d, type=%s" % (self.timestamp, str(self.type))

class EventLogParser(object):
    def __init__(self, app):
        self.__logger = logging.getLogger("viewer.EventLogParser")
        self.__app = app

    def parse(self, filename, progress_callback):
        progress_callback(0)
        buffer = self.__parse_file(filename)
        progress_callback(10)

        # save the length of the buffer for progress updates
        blen = len(buffer)

        # parse the events
        events = {}
        unknown_events = []
        while buffer:
            event = Event(self.__app)
            if not event.deserialize(buffer):
                # NOTE: if any event fails, we bail on everything
                wx.PostEvent(self.__app, ParseErrorEvent(message="Error parsing event!"))
                break

            #self.__logger.debug("Parsed event: %s" % str(event))
            events[event.id] = event
            if event.type.unknown:
                unknown_events.append(event)

            # update the progress (scaled to 90% of the full progress)
            clen = len(buffer)
            progress = ((float(blen - clen) / float(blen)) * 90.0)
            progress_callback(10 + progress)

        if buffer:
            self.__logger.critical("%d bytes remaining in buffer!" % len(buffer))
        self.__logger.info("Found %d unknown events" % len(unknown_events))

        progress_callback(100)
        return events

    def __parse_file(self, filename):
        try:
            with open(filename, "rb") as file:
                return bytearray(file.read())
        except IOError, e:
            wx.PostEvent(self.__app, ParseErrorEvent(message="Unable to open %s: %s" % (filename, e)))
        return bytearray()

class ParserThread(threading.Thread):
    def __init__(self, app, owner, filename):
        threading.Thread.__init__(self)

        self.__app = app
        self.__owner = owner
        self.__filename = filename

    def run(self):
        parser = EventLogParser(self.__app)
        events = parser.parse(self.__filename, self.progress_callback)

        filters = FilterConfig()
        wx.PostEvent(self.__owner, ParseFinishedEvent(events=events, filters=filters))

    def progress_callback(self, progress):
        wx.PostEvent(self.__owner, ParseProgressEvent(filename=self.__filename, progress=progress))

class DetailsDialog(object):
    def __init__(self, app, event):
        self.__app = app
        self.__event = event
        self.__dialog = self.__app.resource.LoadDialog(self.__app.main_frame, "ID_DETAILS_DIALOG")

        self.__load_controls()
        self.__bind_events()

        self.dialog.SetSizerAndFit(self.dialog.GetSizer())

    @property
    def dialog(self):
        return self.__dialog

    def __load_controls(self):
        wx.xrc.XRCCTRL(self.dialog, "ID_DETAILS_EVENT_ID").SetLabel(str(self.__event.id))
        wx.xrc.XRCCTRL(self.dialog, "ID_DETAILS_EVENT_TIMESTAMP").SetLabel(self.__event.timestamp_str)
        wx.xrc.XRCCTRL(self.dialog, "ID_DETAILS_EVENT_TYPE").SetLabel(self.__event.type.type_name)

        # load event type specific controls
        if self.__event.type.type == EventType.UNKNOWN:
            pass

    def __bind_events(self):
        pass

class Filter(object):
    UNKNOWN = 0
    EVENT_TYPE = 1

    # NOTE: these must match the values in the XRC
    FILTER_NAMES = {
        UNKNOWN : "Select a Type...",
        EVENT_TYPE : "Event Type",
    }
    FILTER_NAMES_REV = dict((value, key) for key, value in FILTER_NAMES.iteritems())

    @staticmethod
    def create_filter(type):
        if Filter.EVENT_TYPE == type:
            return EventTypeFilter()
        return Filter()

    @staticmethod
    def type_to_str(type):
        if type in Filter.FILTER_NAMES:
            return Filter.FILTER_NAMES[type]
        return "Unknown"

    @staticmethod
    def type_from_str(type):
        if type in Filter.FILTER_NAMES_REV:
            return Filter.FILTER_NAMES_REV[type]
        return Filter.UNKNOWN

    def __init__(self, type=UNKNOWN):
        self.__type = type

    @property
    def type(self):
        return self.__type

    @property
    def type_name(self):
        return Filter.type_to_str(self.type)

    @property
    def preview(self):
        return ""

    def filter(self, event):
        """ return true if the event passes the filter """
        return True

    def __str__(self):
        return "Unknown Filter Type!"

class EventTypeFilter(Filter):
    def __init__(self):
        Filter.__init__(self, Filter.EVENT_TYPE)

        self.__event_type = EventType.UNKNOWN

    @property
    def preview(self):
        return EventType.type_to_str(self.__event_type)

    def set_event_type(self, type):
        self.__event_type = type
    event_type = property(lambda self: self.__event_type, set_event_type)

    def filter(self, event):
        return self.event_type == event.type.type

    def __str__(self):
        return "EventTypeFilter: type=%s" % (EventType.type_to_str(self.__event_type))

class FilterConfig(Borg):
    # NOTE: these must match the values in the XRC
    MATCH_NONE = 0
    MATCH_ANY = 1
    MATCH_ALL = 2

    __instantiated = False

    def __init__(self):
        if not self.__instantiated:
            self.__filters = []
            self.__match_type = FilterConfig.MATCH_ANY
        self.__instantiated = True

    @property
    def filters(self):
        return self.__filters

    @property
    def filter_count(self):
        return len(self.__filters)

    @property
    def has_filters(self):
        return self.filter_count > 0

    def set_match_type(self, type):
        self.__match_type = type
    match_type = property(lambda self: self.__match_type, set_match_type)

    def clear(self):
        self.__filters = []

    def add_filter(self, filter):
        self.__filters.append(filter)

    def remove_filter(self, idx):
        del self.__filters[idx]

    def filter(self, event):
        """ returns true if the event passes the filters """
        if not self.has_filters:
            return True

        if self.match_type == FilterConfig.MATCH_NONE:
            for filter in self.filters:
                if filter.filter(event):
                    return False
            return True
        elif self.match_type == FilterConfig.MATCH_ANY:
            for filter in self.filters:
                if filter.filter(event):
                    return True
            return False
        elif self.match_type == FilterConfig.MATCH_ALL:
            for filter in self.filters:
                if not filter.filter(event):
                    return False
            return True

        # default allow everything
        return True

class FilterListCtrl(wx.ListCtrl, wx.lib.mixins.listctrl.ListCtrlAutoWidthMixin):
    def __init__(self):
        # XRC subclass mumbo-jumbo
        p = wx.PreListCtrl()
        self.PostCreate(p)

        wx.lib.mixins.listctrl.ListCtrlAutoWidthMixin.__init__(self)

        self.Bind(wx.EVT_WINDOW_CREATE, self.OnCreate)

    def OnCreate(self, evt):
        # XRC subclass mumbo-jumbo
        self.Unbind(wx.EVT_WINDOW_CREATE)

    @property
    def selected_indices(self):
        selected = []
        item = -1
        while True:
            item = self.GetNextSelected(item)
            if item < 0:
                break
            selected.append(item)
        return selected

    def rebuild(self, filters):
        self.ClearAll()
        self.InsertColumn(0, "Filter Name", width=195)
        self.InsertColumn(1, "Preview")
        wx.SafeYield()

        counter = 0
        for filter in filters:
            self.__insert_filter(counter, filter)
            #wx.SafeYield()
            counter += 1

    def append_filter(self, filter):
        self.__insert_filter(self.GetItemCount(), filter)

    def __insert_filter(self, index, filter):
        self.InsertStringItem(index, filter.type_name)
        self.SetStringItem(index, 1, filter.preview)

class XRCAuiNotebook(wx.Notebook):#wx.aui.AuiNotebook):
    def __init__(self):
        # XRC subclass mumbo-jumbo
        p = wx.PreNotebook()
        self.PostCreate(p)
        self.Bind(wx.EVT_WINDOW_CREATE, self.OnCreate)

    def OnCreate(self, evt):
        # XRC subclass mumbo-jumbo
        self.Unbind(wx.EVT_WINDOW_CREATE)

class XRCIntCtrl(wx.TextCtrl):#wx.lib.intctrl.IntCtrl):
    def __init__(self):
        # XRC subclass mumbo-jumbo
        p = wx.PreTextCtrl()
        self.PostCreate(p)
        self.Bind(wx.EVT_WINDOW_CREATE, self.OnCreate)

    def OnCreate(self, evt):
        # XRC subclass mumbo-jumbo
        self.Unbind(wx.EVT_WINDOW_CREATE)

class FilterDialog(object):
    def __init__(self, app):
        self.__logger = logging.getLogger("viewer.FilterDialog")

        self.__app = app
        self.__dialog = self.__app.resource.LoadDialog(self.__app.main_frame, "ID_FILTER_DIALOG")

        self.__load_controls()
        self.__bind_events()

        self.dialog.SetSizerAndFit(self.dialog.GetSizer())

    @property
    def dialog(self):
        return self.__dialog

    def __load_controls(self):
        config = FilterConfig()

        # general controls
        self.__match_type = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_MATCH_TYPE")
        self.__match_type.SetSelection(config.match_type)

        self.__filter_type = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_TYPE")
        self.__filter_type.SetValue(Filter.type_to_str(Filter.UNKNOWN))

        self.__filter_add = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_ADD")
        self.__filter_add.Enable(False)

        self.__filter_remove = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_REMOVE")
        self.__filter_remove.Enable(False)

        self.__filter_clear = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_CLEAR")
        self.__filter_clear.Enable(config.has_filters)

        self.__filter_list = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_LIST")
        self.__filter_list.rebuild(config.filters)

        self.__filter_run = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_RUN")
        self.__filter_run.Enable(self.__app.has_current_page)

        # control panels
        self.__empty_controls = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_CONTROLS_EMPTY")
        self.__event_type_controls = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_CONTROLS_EVENT_TYPE")

        self.__current_controls = self.__empty_controls

        # event type controls
        self.__event_type_type = wx.xrc.XRCCTRL(self.dialog, "ID_FILTER_EVENT_TYPE_TYPE")
        self.__event_type_type.SetValue(EventType.type_to_str(EventType.UNKNOWN))

    def __bind_events(self):
        # control events
        self.__match_type.Bind(wx.EVT_RADIOBOX, self.OnRadioBox)
        self.__filter_type.Bind(wx.EVT_COMBOBOX, self.OnComboBox)
        self.__filter_add.Bind(wx.EVT_BUTTON, self.OnAddFilter)
        self.__filter_remove.Bind(wx.EVT_BUTTON, self.OnRemoveFilter)
        self.__filter_clear.Bind(wx.EVT_BUTTON, self.OnClearFilters)
        self.__filter_run.Bind(wx.EVT_BUTTON, self.OnRunFilters)
        self.dialog.Bind(wx.EVT_BUTTON, self.OnClose, id=wx.xrc.XRCID("wxID_CLOSE"))

        # filter list events
        self.__filter_list.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected)
        self.__filter_list.Bind(wx.EVT_LIST_ITEM_DESELECTED, self.OnItemDeselected)

    def __reset_selected_filter(self):
        self.__filter_type.SetValue(Filter.type_to_str(Filter.UNKNOWN))
        self.__filter_add.Enable(False)
        self.__clear_filter_controls()
        self.__show_empty_controls()

    def __show_filter_options(self, item):
        if Filter.EVENT_TYPE == item:
            self.__show_event_type_controls()
        else:
            self.__show_empty_controls()

    def __show_empty_controls(self):
        self.__current_controls.Show(False)
        self.__empty_controls.Show(True)
        self.__current_controls = self.__empty_controls
        self.dialog.SetSizerAndFit(self.dialog.GetSizer())

    def __show_event_type_controls(self):
        self.__current_controls.Show(False)
        self.__event_type_controls.Show(True)
        self.__current_controls = self.__event_type_controls
        self.dialog.SetSizerAndFit(self.dialog.GetSizer())

    def __clear_filter_controls(self):
        # event type controls
        self.__event_type_type.SetValue(EventType.type_to_str(EventType.UNKNOWN))

    def OnRadioBox(self, evt):
        config = FilterConfig()
        config.match_type = self.__match_type.GetSelection()

    def OnComboBox(self, evt):
        item = self.__filter_type.GetCurrentSelection()
        self.__filter_add.Enable(item > 0)
        self.__clear_filter_controls()
        self.__show_filter_options(item)

    def OnAddFilter(self, evt):
        item = self.__filter_type.GetCurrentSelection()
        if item <= 0:
            return

        filter = Filter.create_filter(item)
        if Filter.EVENT_TYPE == item:
            filter.event_type = EventType.type_from_str(self.__event_type_type.GetValue())

        config = FilterConfig()
        config.add_filter(filter)
        self.__filter_list.append_filter(filter)

        self.__reset_selected_filter()
        self.__filter_clear.Enable(True)
        self.__filter_run.Enable(self.__app.has_current_page)

    def OnRemoveFilter(self, evt):
        config = FilterConfig()

        remove = self.__filter_list.selected_indices
        remove.reverse()
        for item in remove:
            config.remove_filter(item)
            self.__filter_list.DeleteItem(item)

        self.__filter_remove.Enable(self.__filter_list.GetSelectedItemCount() > 0)
        self.__filter_clear.Enable(config.has_filters)
        self.__filter_run.Enable(self.__app.has_current_page)

    def OnClearFilters(self, evt):
        config = FilterConfig()
        config.clear()

        self.__filter_list.DeleteAllItems()
        self.__filter_remove.Enable(False)
        self.__filter_clear.Enable(False)
        self.__filter_run.Enable(self.__app.has_current_page)

    def OnRunFilters(self, evt):
        if self.__app.has_current_page:
            wx.PostEvent(self.__app.current_page, RebuildEventListEvent())
        self.dialog.Close()

    def OnClose(self, evt):
        self.dialog.Close()

    def OnItemSelected(self, evt):
        self.__filter_remove.Enable(self.__filter_list.GetSelectedItemCount() > 0)

    def OnItemDeselected(self, evt):
        self.__filter_remove.Enable(self.__filter_list.GetSelectedItemCount() > 0)

class EventListCtrl(wx.ListCtrl, wx.lib.mixins.listctrl.ListCtrlAutoWidthMixin, wx.lib.mixins.listctrl.ColumnSorterMixin):
    def __init__(self):
        # XRC subclass mumbo-jumbo
        p = wx.PreListCtrl()
        self.PostCreate(p)

        wx.lib.mixins.listctrl.ListCtrlAutoWidthMixin.__init__(self)

        # NOTE: column count must be updated here
        wx.lib.mixins.listctrl.ColumnSorterMixin.__init__(self, 4)

        self.Bind(wx.EVT_WINDOW_CREATE, self.OnCreate)

    def OnCreate(self, evt):
        # XRC subclass mumbo-jumbo
        self.Unbind(wx.EVT_WINDOW_CREATE)

        self.itemDataMap = {}

    def __create_columns(self):
        # NOTE: changing the column count here
        # means changing the column count passed
        # to the ColumnSorterMixin constructor in __init__()!
        self.InsertColumn(0, "Id", width=45)
        self.InsertColumn(1, "Timestamp", width=190)
        self.InsertColumn(2, "Event Type", width=195)
        self.InsertColumn(3, "Preview")

    @property
    def selected_event(self):
        idx = self.GetFirstSelected()
        if idx < 0:
            return None

        data = self.GetItemData(idx)
        if data not in self.itemDataMap:
            return None
        return self.itemDataMap[data]

    def GetListCtrl(self):
        return self

    def rebuild(self, app, events={}, filters=None):
        """ NOTE: filters here is the FilterConfig instance to use.
            We do this because for whatever reason, creating the
            FilterConfig() here causes it to re-instantiate
            and lose all of the previously configured filters """
        self.ClearAll()
        self.__create_columns()
        wx.SafeYield()

        if not events:
            events = self.itemDataMap

        count = len(events)
        self.itemDataMap = events

        filtered = 0
        counter = 0
        current = 0
        for event in events.values():
            percent = (float(current) / float(count)) * 100.0
            app.status = "Processing event %d/%d (%.2f%%), this may take a while..." % (current, count, percent)

            if filters and filters.filter(event):
                self.__insert_event(counter, event)
                #wx.SafeYield()
                counter += 1
            else:
                filtered += 1

            current += 1

        # TODO: we need to refresh the column sizes or something here
        # because the final column doesn't get re-sized properly
        # until the user manually sizes it once

        self.SortListItems(0, -1)
        return filtered

    def __insert_event(self, index, event):
        self.InsertStringItem(index, str(event.id))
        self.SetStringItem(index, 1, event.timestamp_str)
        self.SetStringItem(index, 2, event.type.type_name)
        self.SetStringItem(index, 3, event.type.preview)
        self.SetItemData(index, event.id)

class EventPage(object):
    def __init__(self, app, parent):
        self.__app = app
        self.__panel = self.__app.resource.LoadPanel(parent, "ID_EVENT_PANEL")

        self.__parser_thread = None
        self.__timer_start = 0

        self.__load_controls()
        self.__bind_events()

        self.panel.SetSizerAndFit(self.panel.GetSizer())

    @property
    def panel(self):
        return self.__panel

    def __load_controls(self):
        self.__event_list = wx.xrc.XRCCTRL(self.panel, "ID_EVENT_LIST")
        self.__event_list.rebuild(self.__app)

    def __bind_events(self):
        # list control events
        self.__event_list.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected)
        self.__event_list.Bind(wx.EVT_LIST_ITEM_DESELECTED, self.OnItemDeselected)
        self.__event_list.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.OnItemActivated)
        self.__event_list.Bind(wx.EVT_LIST_ITEM_RIGHT_CLICK, self.OnItemRightClick)

        # custom events
        self.panel.Bind(EVT_PARSE_BEGIN, self.OnParseBegin)
        self.panel.Bind(EVT_PARSE_PROGRESS, self.OnParseProgress)
        self.panel.Bind(EVT_PARSE_FINISHED, self.OnParseFinished)
        self.panel.Bind(EVT_REBUILD_EVENT_LIST, self.OnRebuildEventList)
        self.panel.Bind(EVT_EVENT_DETAILS, self.OnEventDetails)

    def __show_details(self, event):
        if event:
            dlg = DetailsDialog(self.__app, event)
            dlg.dialog.Show()

    def __rebuild_event_list(self, timer_start, events={}, filters=None):
        self.__app.status = "Rebuilding event list (this may take a while..."
        filtered = self.__event_list.rebuild(self.__app, events, filters)
        self.__app.status = "Processed %d events in %.2f seconds (%d filtered)" % (len(events), time.time() - timer_start, filtered)

    def OnParseBegin(self, evt):
        self.__timer_start = time.time()
        self.__parser_thread = ParserThread(self.__app, self.panel, evt.filename)
        self.__parser_thread.start()

    def OnParseProgress(self, evt):
        self.__app.status = "Parsing log file %s: %.2f%%" % (evt.filename, evt.progress)

    def OnParseFinished(self, evt):
        self.__parser_thread.join()
        self.__parser_thread = None

        self.__rebuild_event_list(self.__timer_start, evt.events, evt.filters)

    def OnItemSelected(self, evt):
        self.__app.enable_details(self.__event_list.GetSelectedItemCount() > 0)

    def OnItemDeselected(self, evt):
        self.__app.enable_details(self.__event_list.GetSelectedItemCount() > 0)

    def OnItemActivated(self, evt):
        self.__show_details(self.__event_list.selected_event)

    def OnItemRightClick(self, evt):
        if evt.GetIndex() < 0:
            return
        self.__event_list.PopupMenu(self.__app.event_menu)

    def OnRebuildEventList(self, evt):
        self.__rebuild_event_list(time.time(), filters=FilterConfig())

    def OnEventDetails(self, evt):
        self.__show_details(self.__event_list.selected_event)

class ViewerApp(wx.App):
    TITLE = "Event Viewer"
    SIZE = (1200, 800)

    @property
    def resource(self):
        return self.__resource

    @property
    def main_frame(self):
        return self.__main_frame

    @property
    def event_menu(self):
        return self.__event_menu

    @property
    def current_page(self):
        return self.__event_notebook.GetCurrentPage()

    @property
    def has_current_page(self):
        return None != self.current_page

    status = property(lambda self: self.main_frame.GetStatusText(), lambda self, status: self.main_frame.SetStatusText(status))

    def enable_details(self, enable):
        self.__event_menu.Enable(self.__event_details, enable)

    def __load_controls(self):
        self.__resource = wx.xrc.XmlResource("viewer.xrc")
        self.__main_frame = self.resource.LoadFrame(None, "ID_MAIN_FRAME")
        self.main_frame.SetSize(ViewerApp.SIZE)

        # menus
        self.__event_menu = self.resource.LoadMenu("ID_EVENT_MENU")
        self.main_frame.GetMenuBar().Insert(1, self.event_menu, "&Event")

        # menu items
        self.__file_close = wx.xrc.XRCID("wxID_CLOSE")
        self.__event_details = wx.xrc.XRCID("ID_DETAILS")

        # controls
        self.__event_notebook = wx.xrc.XRCCTRL(self.main_frame, "ID_EVENT_NOTEBOOK")

    def __bind_events(self):
        # menu events
        self.Bind(wx.EVT_MENU, self.OnFileOpen, id=wx.xrc.XRCID("wxID_OPEN"))
        self.Bind(wx.EVT_MENU, self.OnFileClose, id=self.__file_close)
        self.Bind(wx.EVT_MENU, self.OnFileExit, id=wx.xrc.XRCID("wxID_EXIT"))
        self.Bind(wx.EVT_MENU, self.OnEventDetails, id=self.__event_details)
        self.Bind(wx.EVT_MENU, self.OnFilterFilter, id=wx.xrc.XRCID("ID_FILTER"))
        self.Bind(wx.EVT_MENU, self.OnHelpAbout, id=wx.xrc.XRCID("wxID_ABOUT"))

        # custom events
        self.Bind(EVT_PARSE_ERROR, self.OnParseError)

    def __reset(self):
        self.main_frame.SetTitle(ViewerApp.TITLE)
        self.status = ""
        self.main_frame.GetMenuBar().Enable(self.__file_close, False)

    def OnInit(self):
        self.__load_controls()
        self.__bind_events()
        self.__reset()

        self.main_frame.Show()
        return True

    def OnFileOpen(self, evt):
        dlg = wx.FileDialog(self.main_frame, "Open Event Log", wildcard="Event Log (*.evt)|*.evt|All Files (*.*)|*.*",
            style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST)
        if dlg.ShowModal() == wx.ID_OK:
            filename = os.path.join(dlg.GetDirectory(), dlg.GetFilename())

            # clean up the UI
            self.main_frame.SetTitle("%s - %s" % (ViewerApp.TITLE, filename))
            self.status = "Parsing event log %s: 0%%..." % filename
            self.main_frame.GetMenuBar().Enable(self.__file_close, True)

            # add the page to the notebook
            page = EventPage(self, self.__event_notebook)
            self.__event_notebook.AddPage(page.panel, os.path.basename(filename), True)

            # begin parsing
            wx.PostEvent(page.panel, ParseBeginEvent(filename=filename))
        dlg.Destroy()

    def OnFileClose(self, evt):
        # get the currently selected page
        selected = self.__event_notebook.GetSelection()
        if selected < 0:
            return

        # delete it and clean up if it was the last page
        self.__event_notebook.DeletePage(selected)
        if self.__event_notebook.GetPageCount() <= 0:
            self.__reset()

    def OnFileExit(self, evt):
        self.main_frame.Close(True)

    def OnEventDetails(self, evt):
        # post an event to the current page
        # and let it figure out what to do
        if self.has_current_page:
            wx.PostEvent(page, EventDetails())

    def OnFilterFilter(self, evt):
        dlg = FilterDialog(self)
        dlg.dialog.ShowModal()
        dlg.dialog.Destroy()

    def OnHelpAbout(self, evt):
        dlg = wx.MessageDialog(self.main_frame, ViewerApp.TITLE, "About %s" % ViewerApp.TITLE, wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

    def OnParseError(self, evt):
        wx.MessageBox(evt.message, "Parse Error", wx.OK | wx.ICON_ERROR, self)

logger = None

def parse_arguments(argv):
    parser = optparse.OptionParser(version="%prog ", description=ViewerApp.TITLE,
        usage="Usage: viewer.py")
    (options, args) = parser.parse_args(argv[1:])

def initialize_logger():
    global logger

    logging.basicConfig(level=logging.DEBUG, format="%(asctime)s (%(process)d, %(thread)d) %(module)s:%(lineno)d %(levelname)s %(message)s")
    logger = logging.getLogger("viewer")

def main(argv=None):
    global logger

    if not argv:
        argv = sys.argv
    parse_arguments(argv)

    initialize_logger()
    logger.info("%s starting..." % ViewerApp.TITLE)

    app = ViewerApp()
    app.MainLoop()

    return 0

if __name__ == "__main__":
    sys.exit(main())
