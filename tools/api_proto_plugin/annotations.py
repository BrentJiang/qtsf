"""Envoy API annotations."""

from collections import namedtuple

import re

# Key-value annotation regex.
ANNOTATION_REGEX = re.compile('\[#([\w-]+?):(.*?)\]\s?', re.DOTALL)

# Page/section titles with special prefixes in the proto comments
DOC_TITLE_ANNOTATION = 'protodoc-title'

# Not implemented yet annotation on leading comments, leading to insertion of
# warning on field.
NOT_IMPLEMENTED_WARN_ANNOTATION = 'not-implemented-warn'

# Not implemented yet annotation on leading comments, leading to hiding of
# field.
NOT_IMPLEMENTED_HIDE_ANNOTATION = 'not-implemented-hide'

# Comment that allows for easy searching for things that need cleaning up in the next major
# API version.
NEXT_MAJOR_VERSION_ANNOTATION = 'next-major-version'

# Comment. Just used for adding text that will not go into the docs at all.
COMMENT_ANNOTATION = 'comment'

# proto compatibility status.
PROTO_STATUS_ANNOTATION = 'proto-status'

# Where v2 differs from v1..
V2_API_DIFF_ANNOTATION = 'v2-api-diff'

VALID_ANNOTATIONS = set([
    DOC_TITLE_ANNOTATION,
    NOT_IMPLEMENTED_WARN_ANNOTATION,
    NOT_IMPLEMENTED_HIDE_ANNOTATION,
    V2_API_DIFF_ANNOTATION,
    NEXT_MAJOR_VERSION_ANNOTATION,
    COMMENT_ANNOTATION,
    PROTO_STATUS_ANNOTATION,
])

# These can propagate from file scope to message/enum scope (and be overridden).
INHERITED_ANNOTATIONS = set([
    PROTO_STATUS_ANNOTATION,
])


class AnnotationError(Exception):
  """Base error class for the annotations module."""


def ExtractAnnotations(s, inherited_annotations=None):
  """Extract annotations map from a given comment string.

  Args:
    s: string that may contains annotations.
    inherited_annotations: annotation map from file-level inherited annotations
      (or None) if this is a file-level comment.

  Returns:
    Annotation map.
  """
  annotations = {
      k: v for k, v in (inherited_annotations or {}).items() if k in INHERITED_ANNOTATIONS
  }
  # Extract annotations.
  groups = re.findall(ANNOTATION_REGEX, s)
  for group in groups:
    annotation = group[0]
    if annotation not in VALID_ANNOTATIONS:
      raise AnnotationError('Unknown annotation: %s' % annotation)
    annotations[group[0]] = group[1].lstrip()
  return annotations


def WithoutAnnotations(s):
  return re.sub(ANNOTATION_REGEX, '', s)
