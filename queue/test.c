#include "minunit.h"
#include "cqfs.h"

MU_TEST(get_size)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    mu_check(10 == cqfs_get_size(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(get_length)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    mu_check(0 == cqfs_get_length(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(after_init_is_empty)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    mu_check(cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(after_one_push_is_not_empty)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    cqfs_push(&cqfs, 2.3);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    mu_check(1 == cqfs_get_length(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(push_until_full)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 5);

    cqfs_push(&cqfs, 1.2);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_push(&cqfs, 2.3);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_push(&cqfs, 3.4);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_push(&cqfs, 4.5);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_push(&cqfs, 5.6);

    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(cqfs_is_full(&cqfs));

    cqfs_destroy(&cqfs);
}

MU_TEST(pop_unil_empty)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 5);

    cqfs_push(&cqfs, 1.2);
    cqfs_push(&cqfs, 2.3);
    cqfs_push(&cqfs, 3.4);
    cqfs_push(&cqfs, 4.5);
    cqfs_push(&cqfs, 5.6);

    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(cqfs_is_full(&cqfs));

    cqfs_pop(&cqfs);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_pop(&cqfs);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_pop(&cqfs);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_pop(&cqfs);
    mu_check(!cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    cqfs_pop(&cqfs);

    mu_check(cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));

    cqfs_destroy(&cqfs);
}

MU_TEST(after_one_push_and_one_pop_is_empty)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    cqfs_push(&cqfs, 2.3);
    cqfs_pop(&cqfs);
    mu_check(cqfs_is_empty(&cqfs));
    mu_check(!cqfs_is_full(&cqfs));
    mu_check(0 == cqfs_get_length(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(one_push_one_pop_same_element)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    cqfs_push(&cqfs, 2.3);
    mu_assert_double_eq(2.3, cqfs_pop(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST(two_push_two_pop_same_order)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    cqfs_push(&cqfs, 2.3);
    cqfs_push(&cqfs, 3.4);
    mu_assert_double_eq(2.3, cqfs_pop(&cqfs));
    mu_assert_double_eq(3.4, cqfs_pop(&cqfs));
    cqfs_destroy(&cqfs);
}

// just to be sure
MU_TEST(three_push_three_pop_same_order)
{
    CQFS cqfs;
    cqfs_init(&cqfs, 10);
    cqfs_push(&cqfs, 2.3);
    cqfs_push(&cqfs, 3.4);
    cqfs_push(&cqfs, 4.5);
    mu_assert_double_eq(2.3, cqfs_pop(&cqfs));
    mu_assert_double_eq(3.4, cqfs_pop(&cqfs));
    mu_assert_double_eq(4.5, cqfs_pop(&cqfs));
    cqfs_destroy(&cqfs);
}

MU_TEST_SUITE(cqfs_tests)
{
    MU_RUN_TEST(get_size);
    MU_RUN_TEST(get_length);
    MU_RUN_TEST(after_init_is_empty);
    MU_RUN_TEST(push_until_full);
    MU_RUN_TEST(pop_unil_empty);
    MU_RUN_TEST(after_one_push_is_not_empty);
    MU_RUN_TEST(after_one_push_and_one_pop_is_empty);
    MU_RUN_TEST(one_push_one_pop_same_element);
    MU_RUN_TEST(two_push_two_pop_same_order);
    MU_RUN_TEST(three_push_three_pop_same_order);
}

int main(void)
{
    MU_RUN_SUITE(cqfs_tests);
    MU_REPORT();
    return 0;
}