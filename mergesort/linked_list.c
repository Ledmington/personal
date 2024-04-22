#include "linked_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void copy_list2array(list_elem *list, int *v, const unsigned int vsize) {
	list_elem *tmp = list;

	// Actual copy of list's values
	unsigned int i = 0;
	while (tmp != NULL && i < vsize) {
		v[i] = tmp->value;
		i++;
		tmp = tmp->next;
	}
}

list_elem *copy_array2list(int *v, const unsigned int vsize) {
	list_elem *head = NULL;
	list_elem *tmp;

	for (unsigned int i = 0; i < vsize; i++) {
		tmp = (list_elem *)malloc(sizeof(list_elem));
		assert(tmp);
		tmp->value = v[i];
		tmp->next = head;
		head = tmp;
	}
	return head;
}

void print_list(list_elem *list) {
	list_elem *tmp = list;
	while (tmp != NULL) {
		printf("%d, ", tmp->value);
		tmp = tmp->next;
	}
	printf("\n");
}

list_elem *split_list(list_elem *list) {
	if (list == NULL || list->next == NULL) return list;
	list_elem *slow = list->next;
	if (list->next->next == NULL) {
		list->next = NULL;
		return slow;
	}
	list_elem *fast = slow->next;

	while (fast->next != NULL) {
		slow = slow->next;
		fast = fast->next;
		if (fast->next != NULL) {
			fast = fast->next;
		}
	}
	list_elem *tmp = slow->next;
	slow->next = NULL;
	return tmp;
}