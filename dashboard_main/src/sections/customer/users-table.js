import PropTypes from 'prop-types';
import { format } from 'date-fns';
import {
  Avatar,
  Box,
  Button,
  Card,
  Checkbox,
  Stack,
  SvgIcon,
  Table,
  TableBody,
  TableCell,
  TableHead,
  TablePagination,
  TableRow,
  Typography
} from '@mui/material';
import { Scrollbar } from 'src/components/scrollbar';
import { getInitials } from 'src/utils/get-initials';
import PlusIcon from '@heroicons/react/24/solid/PlusIcon';

export const UsersTable = (props) => {
  const {
    count = 0,
    items = [],
    onDeselectAll,
    onDeselectOne,
    onPageChange = () => {},
    onRowsPerPageChange,
    onSelectAll,
    onSelectOne,
    page = 0,
    rowsPerPage = 0,
    selected = [],
    setCommand = () => {},
    auth,
  } = props;

  const selectedSome = (selected.length > 0) && (selected.length < items.length);
  const selectedAll = (items.length > 0) && (selected.length === items.length);

  return (
    <Card>
      <Scrollbar>
        <Box sx={{ minWidth: 800 }}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell padding="checkbox">
                  <Checkbox
                    checked={selectedAll}
                    indeterminate={selectedSome}
                    onChange={(event) => {
                      if (event.target.checked) {
                        onSelectAll?.();
                      } else {
                        onDeselectAll?.();
                      }
                    }}
                  />
                </TableCell>
                <TableCell>
                  Name
                </TableCell>
                <TableCell>
                  Password
                </TableCell>
                <TableCell>
                  RFID Tag
                </TableCell>
                <TableCell>
                  Last Entry
                </TableCell>
                <TableCell>
                  Biometrics
                </TableCell>
                <TableCell>
                  RFID
                </TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {items.map((user) => {
                const isSelected = selected.includes(user.id);

                return (
                  <TableRow
                    hover
                    key={user.id}
                    selected={isSelected}
                  >
                    <TableCell padding="checkbox">
                      <Checkbox
                        checked={isSelected}
                        onChange={(event) => {
                          if (event.target.checked) {
                            onSelectOne?.(user.id);
                          } else {
                            onDeselectOne?.(user.id);
                          }
                        }}
                      />
                    </TableCell>
                    <TableCell>
                      {user.username}
                    </TableCell>
                    <TableCell>
                      {user.password}
                    </TableCell>
                    <TableCell>
                      {user.rfid_tag}
                    </TableCell>
                    <TableCell>
                      {user.AuthLogs.length > 0 ? format(new Date(user.AuthLogs[0].createdAt), 'dd/MM/yyyy HH:mm:ss') : 'N/A' }
                    </TableCell>
                    <TableCell>
                      {user.posicao ? 'Yes' : 'No'}
                      {!user.posicao && (
                        <Button
                          startIcon={(
                            <SvgIcon fontSize="small">
                              <PlusIcon />
                            </SvgIcon>
                          )}
                          variant="text"
                          sx={{ ml: 2 }}
                          onClick={() => {
                            setCommand({
                              currentSensor: 'biometria',
                              isRegistering: true,
                              userId: user.id,
                            }, auth);
                          }}
                        >
                          Add
                        </Button>
                      )}
                    </TableCell>
                  </TableRow>
                );
              })}
            </TableBody>
          </Table>
        </Box>
      </Scrollbar>
      <TablePagination
        component="div"
        count={count}
        onPageChange={onPageChange}
        onRowsPerPageChange={onRowsPerPageChange}
        page={page}
        rowsPerPage={rowsPerPage}
        rowsPerPageOptions={[5, 10, 25]}
      />
    </Card>
  );
};

UsersTable.propTypes = {
  count: PropTypes.number,
  items: PropTypes.array,
  onDeselectAll: PropTypes.func,
  onDeselectOne: PropTypes.func,
  onPageChange: PropTypes.func,
  onRowsPerPageChange: PropTypes.func,
  onSelectAll: PropTypes.func,
  onSelectOne: PropTypes.func,
  page: PropTypes.number,
  rowsPerPage: PropTypes.number,
  selected: PropTypes.array
};
