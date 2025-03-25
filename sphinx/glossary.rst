.. include:: _cdefs.rst

Glossary
========

.. glossary::

   adaptive
      Sorting algorithms known to be *adaptive* take advantage of existing
      order in its input to speed the process up. See `Sorting algorithms`_.

   complexity
      Sorting algorithms efficiency is qualified according to their
      computational and space (i.e., memory) *complexity* using the usual
      `Big O`_ notation. See `Sorting algorithms`_.

   heap
      A tree based data structure satisfying the heap property, stating that
      every element of the tree is larger (max-heap) / smaller (min-heap) than
      any of its descendants if they exists. These are commonly used to
      implement priority queues, heap sort, Dijkstra's shortest path
      algorithm...

   in-place
      Sorting algorithms known to be *in-place* only require a constant amount
      of additional memory space to operate. See `Sorting algorithms`_.

   linked list
      A linear collection of data elements where each element points to the next
      and / or the previous one. These kind of structures allow for efficient
      insertion or removal of elements from any position in the sequence at the
      expense of poor random access performances. See `Linked lists`_.

   online
      Sorting algorithms known to be *online* can sort input elements as they
      receive them, i.e. on-the-fly. See `Sorting algorithms`_.

   out-of-place
      Sorting algorithms that are not |in-place|.

   recursive
      Sorting algorithms logic may be *recursive* or *non-recursive*.
      See `Sorting algorithms`_.

   sorting properties
      A set of attributes that qualifies sorting algorithms. These are
      |adaptive|, |stable|, |in-place|, |online|, |recursive|, time and space
      |complexity|. See `Sorting algorithms`_.

   stable
      Sorting algorithms known to be *stable* do not change the relative order
      of input elements with equal keys. See `Sorting algorithms`_.
