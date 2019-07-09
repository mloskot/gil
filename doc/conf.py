# Boost.GIL configuration file for Sphinx documentation builder.

# This file only contains a selection of the most common options.
# For a full list Sphnx options see the documentation:
# http://www.sphinx-doc.org/en/master/config

import os
import sys

extensions = [
    'sphinx.ext.autosectionlabel'
]
templates_path = ['_templates']
exclude_patterns = ['_build']

highlight_language = 'c++'
autosectionlabel_prefix_document = True

project = 'Boost.GIL'
copyright = '2006 - 2019'
author = 'Lubomir Bourdev, Christian Henning, Hailin Jin, Mateusz Łoskot, Andreas Pokorny, Stefan Seefeld and others'

html_logo = '_static/gil-baner-400x100.png'
html_theme = 'sphinx_rtd_theme' # sphinx_rtd_theme, bizstyle
if html_theme == 'sphinx_rtd_theme':
    html_theme_options = {
        'canonical_url': 'https://boostorg.github.io/gil',
        #'analytics_id': 'UA-XXXXXXX-1', #  Provided by Google in your dashboard
        'display_version': True,
        'logo_only': True,
        'prev_next_buttons_location': 'both',
        'style_external_links': False,
        'style_nav_header_background': 'white',
        #'vcs_pageview_mode': '',
        # Toc options
        'collapse_navigation': True,
        'sticky_navigation': True,
        #'navigation_depth': 4,
        'includehidden': True,
        'titles_only': False
    }
    html_context = {
        'display_github': True,
        'github_user': 'boostorg',
        'github_repo': 'gil',
        'github_version': '/develop/doc/'
    }
else:
    assert html_theme == 'bizstyle'
html_show_copyright = True
html_show_sphinx = False
html_show_sourcelink = False
html_split_index = False
html_use_index = False

htmlhelp_basename = 'BoostGILdoc'
