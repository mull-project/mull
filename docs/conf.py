# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

import re
import datetime
import guzzle_sphinx_theme

# -- Project information -----------------------------------------------------

def project_version():
    with open("../CMakeLists.txt") as f:
        for line in f:
            m = re.search('MULL_VERSION (\d+.\d+.\d+)', line)
            if m:
                return m.group(1)

project = 'Mull'
copyright = '2016-' + str(datetime.datetime.now().year) + ', Mull Project'
author = 'Alex Denisov <alex@lowlevelbits.org>, Stanislav Pankevich <s.pankevich@gmail.com>'

version = project_version()
release = version
print("Generating docs for Mull " + version)

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme_path = guzzle_sphinx_theme.html_theme_path()
extensions.append("guzzle_sphinx_theme")
html_theme = 'guzzle_sphinx_theme'

html_theme_options = {
    "project_nav_name": "Mull",
}
html_sidebars = { '**': ['globaltoc.html', 'searchbox.html'] }

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
html_css_files = [ 'custom.css' ]
master_doc = 'index'