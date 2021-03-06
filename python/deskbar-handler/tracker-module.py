# This deskbar module was ported from deskbar <= 2.18 handler by Marcus Fritzsch

import gnome
import gnomedesktop
import gobject
import re
import sys
import urllib
import string
import time
import cgi
import os.path
import deskbar
import deskbar.core.Utils
import deskbar.interfaces.Module
import deskbar.interfaces.Match
import deskbar.interfaces.Action
from deskbar.core.Utils import is_program_in_path, spawn_async
from deskbar.handlers.actions.OpenWithApplicationAction import \
		OpenWithApplicationAction
from deskbar.handlers.actions.OpenDesktopFileAction import \
		OpenDesktopFileAction
from deskbar.handlers.actions.ShowUrlAction import \
		ShowUrlAction
from deskbar.handlers.actions.ActionsFactory import \
		get_actions_for_uri

import gettext
gettext.install('tracker')


MAX_RESULTS = 10
HANDLERS = ['TrackerSearchToolHandler', 'TrackerLiveSearchHandler']


class TrackerSearchToolMatch (deskbar.interfaces.Match):

	def __init__(self, **kwargs):
		deskbar.interfaces.Match.__init__(self, **kwargs)
		self.add_action (TrackerSearchToolAction (self.get_name ()))
		self._pixbuf = deskbar.core.Utils.load_icon ('tracker')

	def get_hash (self, text=None):
		return 'tst-more-hits-action-'+self.get_name ()

	def get_category (self):
		return 'actions'




class TrackerSearchToolAction (deskbar.interfaces.Action):
	def __init__(self, name):
		deskbar.interfaces.Action.__init__ (self, name)
		self.name = name

	def activate(self, text=None):
		try:
			gobject.spawn_async(['tracker-search-tool', self.name], \
					flags=gobject.SPAWN_SEARCH_PATH)
		except gobject.GError, e:
			print >> sys.stderr, "*** Error when executing tracker-search-tool:", e

	def get_verb(self):
		return _('Search for %s with Tracker Search Tool') % '<b>%(name)s</b>'

	def get_hash (self):
		return 't-s-t:'+self.name

	def get_category (self):
		return 'actions'




class TrackerSearchToolHandler(deskbar.interfaces.Module):
	INFOS = {
			'icon': deskbar.core.Utils.load_icon ('tracker'),
			'name': _('Tracker Search'),
			'description': _('Search with Tracker Search Tool'),
			'version': '0.6.91',
	}

	def __init__(self):
		deskbar.interfaces.Module.__init__(self)

	def query(self, query):
		self._emit_query_ready (query, [TrackerSearchToolMatch(name=query, priority=self.get_priority ())])

	@staticmethod
	def has_requirements ():
		return is_program_in_path ('tracker-search-tool')




#For now description param it's not used
TYPES = {
	'Applications': {
		'description': (_('Launch %s (%s)') % ('<b>%(name)s</b>', '%(app_name)s') ),
		'category': 'actions',
	},

	'GaimConversations': {
		'description': (_('See %s conversation\n%s %s\nfrom %s') % ('<b>%(proto)s</b>', '%(channel)s', '<b>%(conv_to)s</b>', '<i>%(time)s</i>')),
		'category': 'conversations',
	},

	'Emails': {
		'description': (_('Email from %s') % '<i>%(publisher)s</i>' ) + '\n<b>%(title)s</b>',
		'category': 'emails',
		'action': { # more actions for different MUAs
			'key': 'mua', # see TrackerLiveSearchAction.action for a demo
			'Evolution/Email':    'evolution %(uri)s',
			'Modest/Email':    'modest-open %(uri)s',
			'Thunderbird/Email':  'thunderbird -viewtracker %(uri)s',
			'KMail/Email':        'kmail --view %(uri)s',
		},
	},

	'Music': {
		'description': _('Listen to music %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'music',
		#'icon': 'audio-x-generic',
	},

	'Documents': {
		'description': _('See document %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'documents',
	},

	'Development': {
		'description': _('Open file %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'develop',
	},

	'Images': {
		'description': _('View image %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'images',
		'icon': 'image',
	},

	'Videos': {
		'description': _('Watch video  %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'videos',
		#'icon': 'video-x-generic',
	},

	'Folders': {
		'description': _('Open folder %s\n%s') % ('<b>%(name)s</b>', '<i>%(dir)s/%(name)s</i>'),
		'category': 'places',
	},

	'Files': {
		'description': _('Open file %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>'),
		'category': 'files',
	},

	'Extra': {
		'description': _('Search for %s with Tracker Search Tool') % ('<b>%(name)s</b>'),
	},
}




class TrackerLiveSearchMatch (deskbar.interfaces.Match):

	def __init__(self, result, **args):
		deskbar.interfaces.Match.__init__ (self)
		self.result = result
		try:
			desktop = result['desktop']
			del result['desktop']
		except:
			desktop = None

		# Set the match icon
		try:
			self._pixbuf = deskbar.core.Utils.load_icon(TYPES[result['type']]['icon'])
		except:
			if self.result.has_key ('icon'):
				self._pixbuf = deskbar.core.Utils.load_icon_for_desktop_icon (result ['icon'])
			else:
				if not self.result['type'] in ('GaimConversations', 'Emails'):
					try:
						self._pixbuf = deskbar.core.Utils.load_icon ('file://'+result['quoted_uri'])
					except:
						pass # some icons cannot be loaded... (e.g. for non existent file or illegal URI)

		self.add_action (TrackerLiveSearchAction (result, desktop))

		# Add extra default actions where it makes sense
		if not result['type'] in ["Emails", "Applications", "GaimConversations"]:
			try:
				self.add_all_actions (get_actions_for_uri(result['quoted_uri']))
			except:
				print >> sys.stderr, "*** Error when adding all actions for hit %s: %s" % (self.result['uri'], sys.exc_info()[1])

	def get_name (self, text = None):
		return self.get_verb() % self.result

	def get_verb(self):
		try:
			return TYPES[self.result['type']]['description']
		except:
			return _('Open file %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>')

	def get_hash(self, text=None):
		return self.result['uri']

	def get_category (self):
		try:
			return TYPES[self.result['type']]['category']
		except:
			return 'files'




class TrackerLiveSearchAction (deskbar.interfaces.Action):

	def __init__ (self, result, desktop):
		deskbar.interfaces.Action (self)
		self.name = result['name']
		self.desktop = desktop
		self.result = result
		self.init_names (result['uri'])

	def get_name(self, text=None):
		return self.result

	def get_hash(self, text=None):
		if self.result ['type'] == 'Applications':
			# return a name that matches the one returned by the Program handler of deskbar
			return 'generic_' + self.result ['app_basename']
		return self.result['uri']

	def get_verb(self):
		try:
			return TYPES[self.result['type']]['description']
		except:
			return _('Open file %s\nin %s')	% ('<b>%(base)s</b>', '<i>%(dir)s</i>')

	def activate (self, text=None):
		try:
			if TYPES[self.result['type']].has_key('action'):
				if isinstance (TYPES[self.result['type']]['action'], dict):
					try:
						key = TYPES[self.result['type']]['action']['key']
						cmd = TYPES[self.result['type']]['action'][self.result[key]]
					except:
						print >> sys.stderr, "Unknown action for URI %s (Error: %s)" % (self.result['uri'], sys.exc_info()[1])
						return
				else:
					cmd = TYPES[self.result['type']]['action']
				cmd = map(lambda arg : arg % self.result, cmd.split()) # we need this to handle spaces correctly

				print 'Opening Tracker hit with command:', cmd
				deskbar.core.Utils.spawn_async(cmd)
			else:
				if self.desktop:
					self.desktop.launch ([])
				else:
					deskbar.core.Utils.url_show ('file://'+self.result['quoted_uri'])
				print 'Opening Tracker hit:', self.result['quoted_uri']
		except:
			print >> sys.stderr, "*** Could not activate Hit %s: %s" % (self.result['uri'], sys.exc_info()[1])

	def init_names (self, fullpath):
		dirname, filename = os.path.split(fullpath)
		if filename == '': #We had a trailing slash
			dirname, filename = os.path.split(dirname)

		#Reverse-tilde-expansion
		home = os.path.normpath(os.path.expanduser('~'))
		regexp = re.compile(r'^%s(/|$)' % re.escape(home))
		dirname = re.sub(regexp, r'~\1', dirname)

		self.result ['base'] = filename
		self.result ['dir'] = dirname




class TrackerLiveSearchHandler(deskbar.interfaces.Module):

	INFOS = {
			'icon': deskbar.core.Utils.load_icon ('tracker'),
			'name': _('Tracker Live Search'),
			'description': _('Search with Tracker, as you type'),
			'version': '0.6.91',
			'categories': {
			'develop': {
				'name': _('Development Files'),
			},
			'music': {
				'name': _('Music'),
			},
			'images': {
				'name': _('Images'),
			},
			'videos': {
				'name': _('Videos'),
			},
		},
	}

	@staticmethod
	def has_prerequisites ():
		try:
			import dbus
			try :
				if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
					import dbus.glib

				# Check that Tracker can be started via dbus activation, we will have trouble if it's not
				bus = dbus.SessionBus()
				proxy_obj = bus.get_object('org.freedesktop.DBus', '/org/freedesktop/DBus')
				dbus_iface = dbus.Interface(proxy_obj, 'org.freedesktop.DBus')
				activatables = dbus_iface.ListActivatableNames()
				if not 'org.freedesktop.Tracker' in activatables:
					TrackerLiveSearchHandler.INSTRUCTIONS = ('Tracker is not activatable via dbus')
					return False
			except:
				TrackerLiveSearchHandler.INSTRUCTIONS = ('Python dbus.glib bindings not found.')
				return False
			return True
		except:
			TrackerLiveSearchHandler.INSTRUCTIONS = ('Python dbus bindings not found.')
			return False

	def __init__(self):
		deskbar.interfaces.Module.__init__(self)
		# initing on search request, see self.query
		self.tracker = self.tracker_search = self.tracker_keywords = self.tracker_files = self.search_iface = self.keywords_iface = self.files_iface = None
		self.conv_re = re.compile (r'^.*?/logs/([^/]+)/([^/]+)/([^/]+)/(.+?)\.(:?txt|html)$') # all, proto, account, to-whom, time

	def handle_email_hits (self, info, output):
		if len (info) < 5:
			print >> sys.stderr, "*** Hit for Service Emails had incomplete data, ignoring (%s)" % info[0]
			return 0
		output['title'] = info[3]
		output['publisher'] = info[4]
		output['mua'] = info[2]
		return 1

	def handle_conversation_hits (self, info, output):
		output ['uri'] = info [0]
		m = self.conv_re.match (output['uri'])
		output['channel']=_('with')
		output['proto']=output['conv_from']=output['conv_to']=output['time']='' # XXX, never happened during tests
		if m:
			output['proto'] = m.group (1)
			output['conv_from'] = urllib.unquote (m.group (2))
			output['conv_to'] = urllib.unquote (m.group (3))
			output['time'] = time_from_purple_log (m.group (4))
		if output['conv_to'].endswith ('.chat'):
			output['channel'] = _('in channel')
			output['conv_to'] = output['conv_to'].replace ('.chat','')
		if output['proto'] == 'irc':
			nick_server = output['conv_from'].split ('@')
			if len (nick_server) > 1:
				output['conv_to'] = '%s on %s' % (output['conv_to'], nick_server[1])
		return 1

	def handle_application_hits (self, info, output):
		# print info
		#   dbus.Array(
		#   [
		#     dbus.String(u'/usr/share/applications/gksu.desktop'), # TrackerUri  0
		#     dbus.String(u'Applications'),                         # TrackerType 1
		#     dbus.String(u'Application'),                          # DesktopType 2
		#     dbus.String(u'Root Terminal'),                        # DesktopName 3
		#     dbus.String(u'gksu /usr/bin/x-terminal-emulator'),    # DesktopExec 4
		#     dbus.String(u'gksu-root-terminal')                    # DesktopIcon 5
		#   ],
		#   signature=dbus.Signature('s'))
		# Strip %U or whatever arguments in Exec field
		if len (info) < 6:
			print >> sys.stderr, "*** Hit for Service Applications had incomplete data, ignoring (%s)" % info[0]
			return 0
		output['app_name'] = re.sub(r'%\w+', '', info [4]).strip ()
		output['app_basename'] = os.path.basename (output['app_name'])
		output['app_name'] = output['app_name']
		if output['app_basename'] == '': # strange // in app_name, e.g. nautilus burn:///
			output['app_basename'] = output['app_name']
		output['name'] = info [3]
		output['icon'] = info [5]
		desktop = parse_desktop_file (output['uri'])
		if desktop:
			output['desktop'] = desktop
		return 1

	def receive_hits (self, qstring, hits, max):
		matches = []

		for info in hits:
			output = {}

			if len (info) < 2:
				print >> sys.stderr, "*** Hit had incomplete data, ignoring"
				continue

			info = [str (i) for i in info]

			output['uri'] = info[0]
			output['name'] = os.path.basename(output['uri'])
			output['type'] = info[1]
			output['quoted_uri'] = url_quote (info[0], ';?@&=+$,./')

			if not TYPES.has_key(output['type']):
				output['type'] = 'Files'

			if output['type'] == 'Emails':
				if not self.handle_email_hits (info, output):
					continue

			elif output['type'] == 'GaimConversations':
				if not self.handle_conversation_hits (info, output):
					continue

			elif output['type'] == 'Applications':
				if not self.handle_application_hits (info, output):
					continue

			# applications are launched by .desktop file, if not readable: exclude
			if output['type'] != 'Applications' or output.has_key ('desktop'):
				matches.append(TrackerLiveSearchMatch (output))

		if len (matches):
			self._emit_query_ready (qstring, matches)
			print 'Tracker response for %s; %d hits returned, %d shown' % \
					(qstring, len(hits), len(matches))

	def receive_error (self, error):
		print >> sys.stderr, '*** Tracker dbus error:', error

	def query (self, qstring):
		max = MAX_RESULTS

		if self.tracker:
			try: self.tracker.GetStatus ()
			except: self.tracker = None # reconnect
		if not self.tracker:
			try:
				print "Connecting to Tracker (first search or trackerd restarted)"
				import dbus
				bus = dbus.SessionBus()
				self.tracker = bus.get_object('org.freedesktop.Tracker', '/org/freedesktop/Tracker')
				self.tracker_search = bus.get_object('org.freedesktop.Tracker', '/org/freedesktop/Tracker/Search')
				self.search_iface = dbus.Interface(self.tracker_search, 'org.freedesktop.Tracker.Search')
				self.tracker_keywords = bus.get_object('org.freedesktop.Tracker', '/org/freedesktop/Tracker/Keywords')
				self.keywords_iface = dbus.Interface(self.tracker_keywords, 'org.freedesktop.Tracker.Keywords')
				self.tracker_files = bus.get_object('org.freedesktop.Tracker', '/org/freedesktop/Tracker/Files')
				self.files_iface = dbus.Interface(self.tracker_files, 'org.freedesktop.Tracker.Files')
			except:
				print >> sys.stderr, 'DBus connection to tracker failed, check your settings.'
				return
		for service in [key for key in TYPES.iterkeys () if key != 'Extra']:
			print 'Searching %s' % service
			self.search_iface.TextDetailed (-1, service, qstring, 0, max, \
					reply_handler = lambda hits: self.receive_hits(qstring, hits, max),
					error_handler = self.receive_error)
		print 'Tracker query:', qstring




# this code is stolen from the programs handler of deskbar
def parse_desktop_file(desktop, only_if_visible=False):
	try:
		desktop = gnomedesktop.item_new_from_file(desktop, gnomedesktop.LOAD_ONLY_IF_EXISTS)
	except Exception, e:
		print 'Couldn\'t read desktop file:%s:%s' % (desktop, e)
		return None
	if desktop == None or desktop.get_entry_type() != gnomedesktop.TYPE_APPLICATION:
		return None
	if only_if_visible and desktop.get_boolean(gnomedesktop.KEY_NO_DISPLAY):
		return None
	return desktop

def time_from_purple_log (instr):
	try:
		if instr.find ('+') != -1: # new kind of log timestamp...
			return time.strftime ('%c', time.strptime (re.sub (r'\+\d{4}', '', instr), '%Y-%m-%d.%H%M%S%Z'))
		else: # ...from ancient times
			return time.strftime ('%c', time.strptime (instr, '%Y-%m-%d.%H%M%S'))
	except:
		print >> sys.stderr, '*** time parsing for purple chat log failed: %s' % sys.exc_info ()[1]
	return instr

def url_quote (instr, safe = '/'):
	"""A unicode capable quote, see http://bugs.python.org/issue1712522"""
	return ''.join (map (lambda x: x in (safe+string.letters+string.digits) and x or ('%%%02X' % ord(x)), instr.encode ('utf-8')))
