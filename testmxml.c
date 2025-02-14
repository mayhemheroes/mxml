//
// Test program for Mini-XML, a small XML file parsing library.
//
// Usage:
//
//   ./testmxml input.xml [string-output.xml] >stdio-output.xml
//   ./testmxml "<?xml ..." [string-output.xml] >stdio-output.xml
//
// https://www.msweet.org/mxml
//
// Copyright © 2003-2024 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#include "mxml-private.h"
#ifndef _WIN32
#  include <unistd.h>
#endif // !_WIN32
#include <fcntl.h>
#ifndef O_BINARY
#  define O_BINARY 0
#endif // !O_BINARY


//
// Globals...
//

int		event_counts[7];


//
// Local functions...
//

bool		sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data);
mxml_type_t	type_cb(mxml_node_t *node);
const char	*whitespace_cb(mxml_node_t *node, int where);


//
// 'main()' - Main entry for test program.
//

int					// O - Exit status
main(int  argc,				// I - Number of command-line args
     char *argv[])			// I - Command-line args
{
  int			i;		// Looping var
  FILE			*fp;		// File to read
  int			fd;		// File descriptor
  mxml_node_t		*xml,		// <?xml ...?> node
			*tree,		// Element tree
			*node;		// Node which should be in test.xml
  mxml_index_t		*ind;		// XML index
  char			buffer[16384];	// Save string
  static const char	*types[] =	// Strings for node types
			{
			  "MXML_TYPE_ELEMENT",
			  "MXML_TYPE_INTEGER",
			  "MXML_TYPE_OPAQUE",
			  "MXML_TYPE_REAL",
			  "MXML_TYPE_TEXT"
			};


  // Check arguments...
  if (argc != 2 && argc != 3)
  {
    fputs("Usage: testmxml filename.xml [string-output.xml]\n", stderr);
    return (1);
  }

  // Test the basic functionality...
  xml  = mxmlNewXML("1.0");
  tree = mxmlNewElement(xml, "element");

  if (!tree)
  {
    fputs("ERROR: No parent node in basic test.\n", stderr);
    return (1);
  }

  if (tree->type != MXML_TYPE_ELEMENT)
  {
    fprintf(stderr, "ERROR: Parent has type %s (%d), expected MXML_TYPE_ELEMENT.\n", tree->type < MXML_TYPE_ELEMENT || tree->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[tree->type], tree->type);
    mxmlDelete(tree);
    return (1);
  }

  if (strcmp(tree->value.element.name, "element"))
  {
    fprintf(stderr, "ERROR: Parent value is \"%s\", expected \"element\".\n", tree->value.element.name);
    mxmlDelete(tree);
    return (1);
  }

  mxmlNewInteger(tree, 123);
  mxmlNewOpaque(tree, "opaque");
  mxmlNewReal(tree, 123.4);
  mxmlNewText(tree, 1, "text");

  mxmlLoadString(tree, "<group type='string'>string string string</group>", MXML_NO_CALLBACK);
  mxmlLoadString(tree, "<group type='integer'>1 2 3</group>", MXML_INTEGER_CALLBACK);
  mxmlLoadString(tree, "<group type='real'>1.0 2.0 3.0</group>", MXML_REAL_CALLBACK);
  mxmlLoadString(tree, "<group>opaque opaque opaque</group>", MXML_OPAQUE_CALLBACK);
  mxmlLoadString(tree, "<foo><bar><one><two>value<two>value2</two></two></one></bar></foo>", MXML_OPAQUE_CALLBACK);
  mxmlNewCDATA(tree, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n");
  mxmlNewCDATA(tree,
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n");
  mxmlNewCDATA(tree,
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
               "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n");

  node = tree->child;

  if (!node)
  {
    fputs("ERROR: No first child node in basic test.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (node->type != MXML_TYPE_INTEGER)
  {
    fprintf(stderr, "ERROR: First child has type %s (%d), expected MXML_TYPE_INTEGER.\n", node->type < MXML_TYPE_ELEMENT || node->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[node->type], node->type);
    mxmlDelete(tree);
    return (1);
  }

  if (node->value.integer != 123)
  {
    fprintf(stderr, "ERROR: First child value is %ld, expected 123.\n", node->value.integer);
    mxmlDelete(tree);
    return (1);
  }

  node = node->next;

  if (!node)
  {
    fputs("ERROR: No second child node in basic test.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (node->type != MXML_TYPE_OPAQUE)
  {
    fprintf(stderr, "ERROR: Second child has type %s (%d), expected MXML_TYPE_OPAQUE.\n", node->type < MXML_TYPE_ELEMENT || node->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[node->type], node->type);
    mxmlDelete(tree);
    return (1);
  }

  if (!node->value.opaque || strcmp(node->value.opaque, "opaque"))
  {
    fprintf(stderr, "ERROR: Second child value is \"%s\", expected \"opaque\".\n", node->value.opaque ? node->value.opaque : "(null)");
    mxmlDelete(tree);
    return (1);
  }

  node = node->next;

  if (!node)
  {
    fputs("ERROR: No third child node in basic test.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (node->type != MXML_TYPE_REAL)
  {
    fprintf(stderr, "ERROR: Third child has type %s (%d), expected MXML_TYPE_REAL.\n", node->type < MXML_TYPE_ELEMENT || node->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[node->type], node->type);
    mxmlDelete(tree);
    return (1);
  }

  if (node->value.real != 123.4)
  {
    fprintf(stderr, "ERROR: Third child value is %f, expected 123.4.\n", node->value.real);
    mxmlDelete(tree);
    return (1);
  }

  node = node->next;

  if (!node)
  {
    fputs("ERROR: No fourth child node in basic test.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (node->type != MXML_TYPE_TEXT)
  {
    fprintf(stderr, "ERROR: Fourth child has type %s (%d), expected MXML_TYPE_TEXT.\n", node->type < MXML_TYPE_ELEMENT || node->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[node->type], node->type);
    mxmlDelete(tree);
    return (1);
  }

  if (!node->value.text.whitespace || !node->value.text.string || strcmp(node->value.text.string, "text"))
  {
    fprintf(stderr, "ERROR: Fourth child value is %d,\"%s\", expected 1,\"text\".\n", node->value.text.whitespace, node->value.text.string ? node->value.text.string : "(null)");
    mxmlDelete(tree);
    return (1);
  }

  for (i = 0; i < 4; i ++)
  {
    node = node->next;

    if (!node)
    {
      fprintf(stderr, "ERROR: No group #%d child node in basic test.\n", i + 1);
      mxmlDelete(tree);
      return (1);
    }

    if (node->type != MXML_TYPE_ELEMENT)
    {
      fprintf(stderr, "ERROR: Group child #%d has type %s (%d), expected MXML_TYPE_ELEMENT.\n", i + 1, node->type < MXML_TYPE_ELEMENT || node->type > MXML_TYPE_TEXT ? "UNKNOWN" : types[node->type], node->type);
      mxmlDelete(tree);
      return (1);
    }
  }

  // Test mxmlFindPath...
  node = mxmlFindPath(tree, "*/two");
  if (!node)
  {
    fputs("ERROR: Unable to find value for \"*/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }
  else if (node->type != MXML_TYPE_OPAQUE || strcmp(node->value.opaque, "value"))
  {
    fputs("ERROR: Bad value for \"*/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  node = mxmlFindPath(tree, "foo/*/two");
  if (!node)
  {
    fputs("ERROR: Unable to find value for \"foo/*/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }
  else if (node->type != MXML_TYPE_OPAQUE || strcmp(node->value.opaque, "value"))
  {
    fputs("ERROR: Bad value for \"foo/*/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  node = mxmlFindPath(tree, "foo/bar/one/two");
  if (!node)
  {
    fputs("ERROR: Unable to find value for \"foo/bar/one/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }
  else if (node->type != MXML_TYPE_OPAQUE || strcmp(node->value.opaque, "value"))
  {
    fputs("ERROR: Bad value for \"foo/bar/one/two\".\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  // Test indices...
  ind = mxmlIndexNew(tree, NULL, NULL);
  if (!ind)
  {
    fputs("ERROR: Unable to create index of all nodes.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (ind->num_nodes != 10)
  {
    fprintf(stderr, "ERROR: Index of all nodes contains %lu nodes; expected 10.\n", (unsigned long)ind->num_nodes);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexReset(ind);
  if (!mxmlIndexFind(ind, "group", NULL))
  {
    fputs("ERROR: mxmlIndexFind for \"group\" failed.\n", stderr);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexDelete(ind);

  ind = mxmlIndexNew(tree, "group", NULL);
  if (!ind)
  {
    fputs("ERROR: Unable to create index of groups.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (ind->num_nodes != 4)
  {
    fprintf(stderr, "ERROR: Index of groups contains %lu nodes; expected 4.\n", (unsigned long)ind->num_nodes);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexReset(ind);
  if (!mxmlIndexEnum(ind))
  {
    fputs("ERROR: mxmlIndexEnum failed.\n", stderr);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexDelete(ind);

  ind = mxmlIndexNew(tree, NULL, "type");
  if (!ind)
  {
    fputs("ERROR: Unable to create index of type attributes.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (ind->num_nodes != 3)
  {
    fprintf(stderr, "ERROR: Index of type attributes contains %lu nodes; expected 3.\n", (unsigned long)ind->num_nodes);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexReset(ind);
  if (!mxmlIndexFind(ind, NULL, "string"))
  {
    fputs("ERROR: mxmlIndexFind for \"string\" failed.\n", stderr);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexDelete(ind);

  ind = mxmlIndexNew(tree, "group", "type");
  if (!ind)
  {
    fputs("ERROR: Unable to create index of elements and attributes.\n", stderr);
    mxmlDelete(tree);
    return (1);
  }

  if (ind->num_nodes != 3)
  {
    fprintf(stderr, "ERROR: Index of elements and attributes contains %lu nodes; expected 3.\n", (unsigned long)ind->num_nodes);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexReset(ind);
  if (!mxmlIndexFind(ind, "group", "string"))
  {
    fputs("ERROR: mxmlIndexFind for \"string\" failed.\n", stderr);
    mxmlIndexDelete(ind);
    mxmlDelete(tree);
    return (1);
  }

  mxmlIndexDelete(ind);

  // Check the mxmlDelete() works properly...
  for (i = 0; i < 12; i ++)
  {
    if (tree->child)
    {
      mxmlDelete(tree->child);
    }
    else
    {
      fprintf(stderr, "ERROR: Child pointer prematurely NULL on child #%d\n", i + 1);
      mxmlDelete(tree);
      return (1);
    }
  }

  if (tree->child)
  {
    fputs("ERROR: Child pointer not NULL after deleting all children.\n", stderr);
    return (1);
  }

  if (tree->last_child)
  {
    fputs("ERROR: Last child pointer not NULL after deleting all children.\n", stderr);
    return (1);
  }

  mxmlDelete(xml);

  // Open the file/string using the default (MXML_NO_CALLBACK) callback...
  if (argv[1][0] == '<')
  {
    xml = mxmlLoadString(NULL, argv[1], MXML_NO_CALLBACK);
  }
  else if ((fp = fopen(argv[1], "rb")) == NULL)
  {
    perror(argv[1]);
    return (1);
  }
  else
  {
    // Read the file...
    xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

    fclose(fp);
  }

  if (!xml)
  {
    fputs("Unable to read XML file with default callback.\n", stderr);
    return (1);
  }

  if (!strcmp(argv[1], "test.xml"))
  {
    const char	*text;			// Text value

    // Verify that mxmlFindElement() and indirectly mxmlWalkNext() work properly...
    if ((node = mxmlFindPath(xml, "group/option/keyword")) == NULL)
    {
      fputs("Unable to find group/option/keyword element in XML tree.\n", stderr);
      mxmlDelete(tree);
      return (1);
    }

    if (node->type != MXML_TYPE_TEXT)
    {
      fputs("No child node of group/option/keyword.\n", stderr);
      mxmlSaveFile(xml, stderr, MXML_NO_CALLBACK);
      mxmlDelete(xml);
      return (1);
    }

    if ((text = mxmlGetText(node, NULL)) == NULL || strcmp(text, "InputSlot"))
    {
      fprintf(stderr, "Child node of group/option/value has value \"%s\" instead of \"InputSlot\".\n", text ? text : "(null)");
      mxmlDelete(xml);
      return (1);
    }
  }

  mxmlDelete(xml);

  // Open the file...
  if (argv[1][0] == '<')
  {
    xml = mxmlLoadString(NULL, argv[1], type_cb);
  }
  else if ((fp = fopen(argv[1], "rb")) == NULL)
  {
    perror(argv[1]);
    return (1);
  }
  else
  {
    // Read the file...
    xml = mxmlLoadFile(NULL, fp, type_cb);

    fclose(fp);
  }

  if (!xml)
  {
    fputs("Unable to read XML file.\n", stderr);
    return (1);
  }

  if (!strcmp(argv[1], "test.xml"))
  {
    // Verify that mxmlFindElement() and indirectly mxmlWalkNext() work properly...
    if ((node = mxmlFindElement(xml, xml, "choice", NULL, NULL, MXML_DESCEND)) == NULL)
    {
      fputs("Unable to find first <choice> element in XML tree.\n", stderr);
      mxmlDelete(tree);
      return (1);
    }

    if (!mxmlFindElement(node, xml, "choice", NULL, NULL, MXML_NO_DESCEND))
    {
      fputs("Unable to find second <choice> element in XML tree.\n", stderr);
      mxmlDelete(tree);
      return (1);
    }
  }

  // Print the XML tree...
  mxmlSaveFile(xml, stdout, whitespace_cb);

  // Save the XML tree to a string and print it...
  if (mxmlSaveString(xml, buffer, sizeof(buffer), whitespace_cb) > 0)
  {
    if (argc == 3)
    {
      fp = fopen(argv[2], "w");
      fputs(buffer, fp);
      fclose(fp);
    }
  }

  // Delete the tree...
  mxmlDelete(xml);

  // Read from/write to file descriptors...
  if (argv[1][0] != '<')
  {
    // Open the file again...
    if ((fd = open(argv[1], O_RDONLY | O_BINARY)) < 0)
    {
      perror(argv[1]);
      return (1);
    }

    // Read the file...
    xml = mxmlLoadFd(NULL, fd, type_cb);

    close(fd);

    // Create filename.xmlfd...
    snprintf(buffer, sizeof(buffer), "%sfd", argv[1]);

    if ((fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666)) < 0)
    {
      perror(buffer);
      mxmlDelete(tree);
      return (1);
    }

    // Write the file...
    mxmlSaveFd(xml, fd, whitespace_cb);

    close(fd);

    // Delete the tree...
    mxmlDelete(xml);
  }

  // Test SAX methods...
  memset(event_counts, 0, sizeof(event_counts));

  if (argv[1][0] == '<')
  {
    mxmlRelease(mxmlSAXLoadString(NULL, argv[1], type_cb, sax_cb, NULL));
  }
  else if ((fp = fopen(argv[1], "rb")) == NULL)
  {
    perror(argv[1]);
    return (1);
  }
  else
  {
    // Read the file...
    mxmlRelease(mxmlSAXLoadFile(NULL, fp, type_cb, sax_cb, NULL));

    fclose(fp);
  }

  if (!strcmp(argv[1], "test.xml"))
  {
    if (event_counts[MXML_SAX_EVENT_CDATA] != 1)
    {
      fprintf(stderr, "MXML_SAX_EVENT_CDATA seen %d times, expected 1 times.\n", event_counts[MXML_SAX_EVENT_CDATA]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_COMMENT] != 1)
    {
      fprintf(stderr, "MXML_SAX_EVENT_COMMENT seen %d times, expected 1 times.\n", event_counts[MXML_SAX_EVENT_COMMENT]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_DATA] != 61)
    {
      fprintf(stderr, "MXML_SAX_EVENT_DATA seen %d times, expected 61 times.\n", event_counts[MXML_SAX_EVENT_DATA]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_DECLARATION] != 0)
    {
      fprintf(stderr, "MXML_SAX_EVENT_DECLARATION seen %d times, expected 0 times.\n", event_counts[MXML_SAX_EVENT_DECLARATION]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_DIRECTIVE] != 1)
    {
      fprintf(stderr, "MXML_SAX_EVENT_DIRECTIVE seen %d times, expected 1 times.\n", event_counts[MXML_SAX_EVENT_DIRECTIVE]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_ELEMENT_CLOSE] != 20)
    {
      fprintf(stderr, "MXML_SAX_EVENT_ELEMENT_CLOSE seen %d times, expected 20 times.\n", event_counts[MXML_SAX_EVENT_ELEMENT_CLOSE]);
      return (1);
    }

    if (event_counts[MXML_SAX_EVENT_ELEMENT_OPEN] != 20)
    {
      fprintf(stderr, "MXML_SAX_EVENT_ELEMENT_OPEN seen %d times, expected 20 times.\n", event_counts[MXML_SAX_EVENT_ELEMENT_OPEN]);
      return (1);
    }
  }

#ifndef _WIN32
  // Debug hooks...
  if (getenv("TEST_DELAY") != NULL)
    sleep(atoi(getenv("TEST_DELAY")));

#  ifdef __APPLE__
  if (getenv("TEST_LEAKS") != NULL)
  {
    char command[1024];

    snprintf(command, sizeof(command), "leaks %d", (int)getpid());
    if (system(command))
      puts("Unable to check for leaks.");
  }
#  endif // __APPLE__
#endif // !_WIN32

  // Return...
  return (0);
}


//
// 'sax_cb()' - Process nodes via SAX.
//

bool					// O - `true` to continue, `false` to stop
sax_cb(mxml_node_t      *node,		// I - Current node
       mxml_sax_event_t event,		// I - SAX event
       void             *data)		// I - SAX user data
{
  static const char * const events[] =	// Events
  {
    "MXML_SAX_EVENT_CDATA",			// CDATA node
    "MXML_SAX_EVENT_COMMENT",			// Comment node
    "MXML_SAX_EVENT_DATA",			// Data node
    "MXML_SAX_EVENT_DECLARATION",		// Declaration node
    "MXML_SAX_EVENT_DIRECTIVE",			// Processing directive node
    "MXML_SAX_EVENT_ELEMENT_CLOSE",		// Element closed
    "MXML_SAX_EVENT_ELEMENT_OPEN"		// Element opened
  };


  (void)data;

  // This SAX callback just counts the different events.
  if (!node)
    fprintf(stderr, "ERROR: SAX callback for event %s has NULL node.\n", events[event]);

  event_counts[event] ++;

  return (true);
}


//
// 'type_cb()' - XML data type callback for mxmlLoadFile()...
//

mxml_type_t				// O - Data type
type_cb(mxml_node_t *node)		// I - Element node
{
  const char	*type;			// Type string


  // You can lookup attributes and/or use the element name, hierarchy, etc...
  if ((type = mxmlElementGetAttr(node, "type")) == NULL)
    type = node->value.element.name;

  if (!strcmp(type, "integer"))
    return (MXML_TYPE_INTEGER);
  else if (!strcmp(type, "opaque") || !strcmp(type, "pre"))
    return (MXML_TYPE_OPAQUE);
  else if (!strcmp(type, "real"))
    return (MXML_TYPE_REAL);
  else
    return (MXML_TYPE_TEXT);
}


//
// 'whitespace_cb()' - Let the mxmlSaveFile() function know when to insert
//                     newlines and tabs...
//

const char *				// O - Whitespace string or NULL
whitespace_cb(mxml_node_t *node,	// I - Element node
              int         where)	// I - Open or close tag?
{
  mxml_node_t	*parent;		// Parent node
  int		level;			// Indentation level
  const char	*name;			// Name of element
  static const char *tabs = "\t\t\t\t\t\t\t\t";
					// Tabs for indentation


  // We can conditionally break to a new line before or after any element.
  // These are just common HTML elements...
  if ((name = mxmlGetElement(node)) == NULL)
    name = "";

  if (!strcmp(name, "html") || !strcmp(name, "head") || !strcmp(name, "body") || !strcmp(name, "pre") || !strcmp(name, "p") || !strcmp(name, "h1") || !strcmp(name, "h2") || !strcmp(name, "h3") || !strcmp(name, "h4") || !strcmp(name, "h5") || !strcmp(name, "h6"))
  {
    // Newlines before open and after close...
    if (where == MXML_WS_BEFORE_OPEN || where == MXML_WS_AFTER_CLOSE)
      return ("\n");
  }
  else if (!strcmp(name, "dl") || !strcmp(name, "ol") || !strcmp(name, "ul"))
  {
    // Put a newline before and after list elements...
    return ("\n");
  }
  else if (!strcmp(name, "dd") || !strcmp(name, "dt") || !strcmp(name, "li"))
  {
    // Put a tab before <li>'s, <dd>'s, and <dt>'s, and a newline after them...
    if (where == MXML_WS_BEFORE_OPEN)
      return ("\t");
    else if (where == MXML_WS_AFTER_CLOSE)
      return ("\n");
  }
  else if (mxmlGetType(node) == MXML_TYPE_DIRECTIVE)
  {
    if (where == MXML_WS_AFTER_OPEN)
      return ("\n");
    else
      return (NULL);
  }
  else if (where == MXML_WS_BEFORE_OPEN || ((!strcmp(name, "choice") || !strcmp(name, "option")) && where == MXML_WS_BEFORE_CLOSE))
  {
    for (level = -1, parent = node->parent; parent; level ++, parent = parent->parent);

    if (level > 8)
      level = 8;
    else if (level < 0)
      level = 0;

    return (tabs + 8 - level);
  }
  else if (where == MXML_WS_AFTER_CLOSE || ((!strcmp(name, "group") || !strcmp(name, "option") || !strcmp(name, "choice")) && where == MXML_WS_AFTER_OPEN))
    return ("\n");
  else if (where == MXML_WS_AFTER_OPEN && !mxmlGetFirstChild(node))
    return ("\n");

  // Return NULL for no added whitespace...
  return (NULL);
}
