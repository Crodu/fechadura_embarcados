import { useCallback, useEffect, useMemo, useState } from 'react';
import Head from 'next/head';
import { sub, subDays, subHours } from 'date-fns';
import ArrowDownOnSquareIcon from '@heroicons/react/24/solid/ArrowDownOnSquareIcon';
import ArrowUpOnSquareIcon from '@heroicons/react/24/solid/ArrowUpOnSquareIcon';
import PlusIcon from '@heroicons/react/24/solid/PlusIcon';
import { Box, Button, Container, Stack, SvgIcon, Typography } from '@mui/material';
import { useSelection } from 'src/hooks/use-selection';
import { Layout as DashboardLayout } from 'src/layouts/dashboard/layout';
import { UsersTable } from 'src/sections/customer/users-table';
import { CustomersSearch } from 'src/sections/customer/customers-search';
import { applyPagination } from 'src/utils/apply-pagination';
import { BACKEND_URL, setCommand } from 'src/contexts/api';
import { ModalAdd } from 'src/sections/customer/modal-add';
import { useAuth } from 'src/hooks/use-auth';

const now = new Date();

const useCustomerIds = (customers) => {
  return useMemo(
    () => {
      return customers.map((customer) => customer.id);
    },
    [customers]
  );
};

const Page = () => {
  const [page, setPage] = useState(0);
  const [rowsPerPage, setRowsPerPage] = useState(5);
  const [users, setUsers] = useState([]);
  const [open, setOpen] = useState(false);
  const [submitting, setSubmitting] = useState(false);

  const auth = useAuth();

  useEffect(() => {
    const fetchUsers = async () => {
      const response = await fetch(BACKEND_URL + '/users');
      const users = await response.json();
      setUsers(users);
    };
    fetchUsers();
  }, [submitting]);

  const deleteUser = async (id) => {
    const response = await fetch(`${BACKEND_URL}/users/`, {
      method: 'DELETE',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ userId: id })
    });
  }

  const useCustomers = (page, rowsPerPage) => {
    return useMemo(
      () => {
        return applyPagination(users, page, rowsPerPage);
      },
      [page, rowsPerPage]
    );
  };

  const customers = useCustomers(page, rowsPerPage);
  const customersIds = useCustomerIds(customers);
  const customersSelection = useSelection(customersIds);

  const handlePageChange = useCallback(
    (event, value) => {
      setPage(value);
    },
    []
  );

  const handleRowsPerPageChange = useCallback(
    (event) => {
      setRowsPerPage(event.target.value);
    },
    []
  );

  return (
    <>
      <Head>
        <title>
          Users
        </title>
      </Head>
      <Box
        component="main"
        sx={{
          flexGrow: 1,
          py: 8
        }}
      >
        <Container maxWidth="xl">
          <Stack spacing={3}>
            <Stack
              direction="row"
              justifyContent="space-between"
              spacing={4}
            >
              <Stack spacing={1}>
                <Typography variant="h4">
                  Users
                </Typography>
              </Stack>
              <div>
                <Button
                  startIcon={(
                    <SvgIcon fontSize="small">
                      <PlusIcon />
                    </SvgIcon>
                  )}
                  variant="contained"
                  onClick={() => setOpen(true)}
                >
                  Add
                </Button>
              </div>
            </Stack>
            <CustomersSearch />
            <UsersTable
              count={users.length}
              items={users}
              onDeselectAll={customersSelection.handleDeselectAll}
              onDeselectOne={customersSelection.handleDeselectOne}
              onPageChange={handlePageChange}
              onRowsPerPageChange={handleRowsPerPageChange}
              onSelectAll={customersSelection.handleSelectAll}
              onSelectOne={customersSelection.handleSelectOne}
              page={page}
              rowsPerPage={rowsPerPage}
              selected={customersSelection.selected}
              setCommand={setCommand}
              auth={auth}
              onDelete={deleteUser}
            />
          </Stack>
        </Container>
        <ModalAdd 
          open={open}
          onSubmit={() => setSubmitting(submitting => !submitting)}
          setOpen={setOpen}
          users={users}
        />
      </Box>
    </>
  );
};

Page.getLayout = (page) => (
  <DashboardLayout>
    {page}
  </DashboardLayout>
);

export default Page;
