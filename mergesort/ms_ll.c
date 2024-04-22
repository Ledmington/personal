#include "ms_ll.h"

#include <stddef.h>

list_elem *main_chain(list_elem *first, list_elem *second) {
	list_elem *head, *tmp;

	if (first->value < second->value) {
		head = first;
	} else {
		head = second;
		second = first;
		first = head;
	}

	// Merging second into first
	while (first != NULL && second != NULL) {
		// Inserting first element of second
		if (first->next != NULL && second->value > first->value &&
			second->value < first->next->value) {
			tmp = second->next;
			second->next = first->next;
			first->next = second;
			second = tmp;
		} else {
			tmp = first;
			first = first->next;
		}
	}

	if (first == NULL) {
		tmp->next = second;
	}

	return head;
}

list_elem *swapping_chains(list_elem *first, list_elem *second) {
	list_elem *head, *tmp;

	if (first->value < second->value) {
		head = first;
	} else {
		head = second;
		second = first;
		first = head;
	}

	// Merging second into first
	while (first != NULL && second != NULL) {
		// Inserting first element of second
		if (first->next != NULL && second->value > first->value &&
			second->value < first->next->value) {
			tmp = first->next;
			first->next = second;
			second = tmp;
			first = first->next;
		} else {
			tmp = first;
			first = first->next;
		}
	}

	if (first == NULL) {
		tmp->next = second;
	}

	return head;
}

list_elem *stream_merge(list_elem *first, list_elem *second) {
	list_elem *head, *tmp;

	if (first->value < second->value) {
		head = first;
		first = first->next;
	} else {
		head = second;
		second = second->next;
	}

	tmp = head;

	while (first != NULL && second != NULL) {
		if (first->value < second->value) {
			tmp->next = first;
			first = first->next;
		} else {
			tmp->next = second;
			second = second->next;
		}
		tmp = tmp->next;
	}

	if (first == NULL) {
		tmp->next = second;
	} else if (second == NULL) {
		tmp->next = first;
	}

	return head;
}

list_elem *merge_sort_ll_main_chain(list_elem *list) {
	if (list != NULL && list->next != NULL) {
		list_elem *middle = split_list(list);

		list = merge_sort_ll_main_chain(list);
		middle = merge_sort_ll_main_chain(middle);

		list = main_chain(list, middle);
	}
	return list;
}

list_elem *merge_sort_ll_swap(list_elem *list) {
	if (list != NULL && list->next != NULL) {
		list_elem *middle = split_list(list);

		list = merge_sort_ll_swap(list);
		middle = merge_sort_ll_swap(middle);

		list = swapping_chains(list, middle);
	}
	return list;
}

list_elem *merge_sort_ll_stream(list_elem *list) {
	if (list != NULL && list->next != NULL) {
		list_elem *middle = split_list(list);

		list = merge_sort_ll_stream(list);
		middle = merge_sort_ll_stream(middle);

		list = stream_merge(list, middle);
	}
	return list;
}